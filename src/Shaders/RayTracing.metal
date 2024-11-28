#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

// Math constants
constant float PI = 3.1415926535897932384626433832795;
constant float TAU = PI * 2;
constant float PI_INVERSE = 1 / PI;

// Object constants
constexpr sampler sampler2d(address::clamp_to_edge, filter::linear); 

struct VertexAttributes {
	float4 color;									// {r, g, b, w}
	float2 texture;								// {x, y}
	float3 normal;								// v{x, y, z}
};

struct Submesh
{
  constant uint32_t* indices;					// Indices pointing at the packed vertices
	texture2d<float> texture;
	bool textured;
	bool emissive;
};

struct Mesh
{
	constant packed_float3* vertices;				// Vertices packed: XYZXYZ...
  constant VertexAttributes* attributes;	// Attributes of the vertices
	constant Submesh* submeshes;						// Submeshes related to the mesh
};

struct Scene
{
	constant Mesh* meshes;						// All meshes related to all models
};

struct RTTransform {
	float4x4 mProjection;
	float4x4 mView;
  float4x4 mInverseProjection;
  float4x4 mInverseView;
	float3 rayOrigin;
};

struct RTMaterial {
	float3 color;
	float3 roughness;
	float3 metallic;
};

// PCG Hash for random number generation
uint32_t pcg_hash(thread uint32_t input) {
	uint32_t state = input * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

// Float is randomly distributed between 0 and 1
float rand(thread uint32_t& seed) {
	seed = pcg_hash(seed);
	return (float)seed / (float)0xffffffff;
}

// Uniformly distributed probability density function (PDF)
// Normalizes float between -1 <= n <= 1, then normalizes so vec3 sums to 1.
float3 uniform_pdf(thread uint32_t& seed) {
	return normalize(
		float3(
			rand(seed) * 2 - 1, 
			rand(seed) * 2 - 1, 
			rand(seed) * 2 - 1
		)
	);
}

// Struct required for reservoir sampling
struct Reservoir {
	float w_sum = 0;					// sum of weights
	float m = 0;							// number of samples
	float w = 0;							// weight
	float3 y = float3(0.0f);	// chosen sample (ray direction)

	void update(
		thread float3& sample, 
		thread float& weight,
		thread uint32_t& seed
	) {
		w_sum += weight;
		m += 1;
		float random = rand(seed);
		if (random <= (weight / w_sum)) {
			y = sample;
			w = weight;
		}
	}
};

// Cosine: decrease light intensity based on the angle between the normal and outgoing light direction (wi).
float cos(
	thread float3& wi,
	thread float3& normal
) {
	return max(0.001, saturate(dot(wi, normal)));
}

// Inverse square: light intensity inversely proportional to distance.
float cos_inverse_square(
	thread float& distance, 
	thread float3& wi, 
	thread float3& normal
) {
	float cosine = cos(wi, normal);
	float result = cosine / abs(distance * distance);
	return result;
}

// Check whether there is direct visibility between two points
float visibility_check() {
	return 1;
}

// Calculate the intensity of a vector based on its constituent components excluding the w term
float intensity(
	thread float4& vector
) {
	return (vector.x + vector.y + vector.z) / 3;
}

raytracing::ray build_ray(
	constant float3& resolution,
	constant RTTransform &transform,
	uint2 gid
) {
	// Camera point needs to be within world space:
	// -1 >= x >= 1 :: Normalized
	// -1 >= y >= 1 :: We want to scale y in opposite direction for viewport coordinates
	// -1 >= z >= 0 :: The camera is pointed towards -z axis, as we use right handed
	float3 pixel = float3(float2(gid), 1.0f);
	pixel.xy /= resolution.xy;
	pixel = pixel * 2 - 1;
	pixel *= float3(1, -1, 1);

	// Projection transformations
	float4 projTransform = transform.mInverseProjection * float4(pixel, 1);
	float4 projTransformN = float4(normalize(projTransform.xyz / projTransform.w), 0.0f);
	float3 rayDirection = normalize(transform.mView * projTransformN).xyz;

	raytracing::ray r;
	r.origin = transform.rayOrigin;
	r.direction = rayDirection;
	r.min_distance = 0.1f;
	r.max_distance = FLT_MAX;
	return r;
}

void transport(
	thread raytracing::ray& r,
	thread raytracing::instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	thread int bounces,
	thread float4& color,
	thread uint32_t& seed,
	thread bool unshadowed_light_contribution,
	thread float3& normal,
	thread bool& terminate_flag,
	thread bool inverse_square
) {
	
	raytracing::intersector<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersector;	
	intersector.assume_geometry_type(raytracing::geometry_type::triangle);
	raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersection;

	// Contribution is set to 1 and will decrease over time, with the amount of bounces
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 sky_color = float4(.4f, .5f, .6f, 1.0f);

	for (int i = 1; i <= bounces; i++) {
		// Verify if ray has intersected the geometry
		intersection = intersector.intersect(r, structure, 0xFF);

		// If our ray does not hit, terminate early.
		if (intersection.type == raytracing::intersection_type::none) { 
			color += contribution * sky_color;
			terminate_flag = (i == 1);
			return; 
		} 
		if (intersection.type == raytracing::intersection_type::triangle) {

			// Look up the data belonging to the intersection in the scene
			// This requires a bindless setup				
			Mesh mesh = scene->meshes[intersection.instance_id];
			Submesh submesh = mesh.submeshes[intersection.geometry_id];
			texture2d<float> texture = submesh.texture;

			float2 bary_2d = intersection.triangle_barycentric_coord;
			float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

			uint32_t tri_index_1 = submesh.indices[intersection.primitive_id * 3 + 0];
			uint32_t tri_index_2 = submesh.indices[intersection.primitive_id * 3 + 1];
			uint32_t tri_index_3 = submesh.indices[intersection.primitive_id * 3 + 2];

			VertexAttributes attr_1 = mesh.attributes[tri_index_1];
			VertexAttributes attr_2 = mesh.attributes[tri_index_2];
			VertexAttributes attr_3 = mesh.attributes[tri_index_3];
			//const device VertexAttributes* attr = (const device VertexAttributes*) intersection.primitive_data;

			// Calculate variables needed to solve the rendering equation
			// We assume for now that the surface is perfectly reflective.
			// You can chance this for materials and so forth
			// Called: BSDF: bi-directional scattering distribution function.
			normal = (attr_1.normal * bary_3d.x) + (attr_2.normal * bary_3d.y) + (attr_3.normal * bary_3d.z);
			normal = normalize((intersection.object_to_world_transform * float4(normal, 0.0f)).xyz);
				
			seed = intensity(contribution) + gid.x * (tri_index_1 * bary_2d.x * i) + gid.y * (tri_index_3 * bary_2d.y);
			r.origin = r.origin + r.direction * intersection.distance; 
			float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .2);
			r.direction = reflect(r.direction, jittered_normal);

			float3 wi = normalize(r.direction);
			float wi_dot_n = (inverse_square) ? cos_inverse_square(intersection.distance, wi, normal) : cos(wi, normal);

			// Calculate all color contributions; use texture, emission if there is one
			float2 tx_point = (attr_1.texture * bary_3d.x) + (attr_2.texture * bary_3d.y) + (attr_3.texture * bary_3d.z);
			float4 wo_color = (submesh.textured) ? texture.sample(sampler2d, tx_point) : attr_1.color;
			
			contribution *= wo_color * wi_dot_n;
			// color *= (unshadowed_light_contribution) ? 1 : visibility_check();
			color += submesh.emissive * wo_color;
		}
	}
	color += contribution * sky_color;
}

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer								[[ texture(0) ]],
	constant float3& resolution														[[ buffer(0)	]],
	constant float3& lightDir															[[ buffer(1)	]],
	constant RTTransform& transform												[[ buffer(2)	]],
	raytracing::instance_acceleration_structure structure	[[ buffer(3)	]],
	constant Scene* scene																	[[ buffer(4)	]],
	uint2 gid																							[[ thread_position_in_grid	]] 
) {
	
	// Initialize default parameters
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	thread uint32_t seed = 0;
	thread int bounces = 3;
	thread float3 normal = float3(0.0f);

	// Check if instance acceleration structure was built succesfully
	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, gid);
		return;
	}

	// Initialize ReSTIR variables (later to reconsider the memory scope)
	// N good samples defined in constant address space, maybe there is a better way of doing things
	thread int m = 8;									// Number of bad samples
	thread int n = 1;									// Number of good samples
	
	// In the future we need to pass it in as an argument to the kernel shader
	// Otherwise you have to instantiate them here
	Reservoir r1;

	// Build ray. Ray shoots out from point (gid). Camera is a grid, point is a coordinate on the grid. 
	raytracing::ray r = build_ray(resolution, transform, gid);
	raytracing::ray x;
	
	// Shoot initial ray from the camera into the scene. 
	// This will set the ray, color and seed by reference.
	bool terminate_flag = false;
	transport(r, structure, scene, gid, bounces, color, seed, true, normal, terminate_flag, false);
	if (terminate_flag) {
		buffer.write(color, gid);
		return;
	}
	x = r;
	
	// ReSTIR GI implementation
	for (int i = 0; i < m; i += 1) {

		// 1. Calculate the samples from the uniform pdf.
		// All samples in a uniform have equal weights, so we don't have to calculate them. The sum of weights would be 1.
		//float3 pdf_sample = normalize(normal + uniform_pdf(seed) * 1); 
		float3 pdf_sample = uniform_pdf(seed);	
		// 2. Calculate the weights of the complex pdf through the unshadowed light contribution.
		// For the complex pdf, we take the color contribution as the weights, required for the re-sample. 
		float4 sample_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
		x.direction = pdf_sample;
		x.origin = r.origin;
		transport(x, structure, scene, gid, 2, sample_color, seed, true, normal, terminate_flag, true);
		float pdf_weight = intensity(sample_color);

		// 3. Build the complex pdf to re-sample the samples from.
		// Sample color intensity are weights for the complex pdf.
		r1.update(pdf_sample, pdf_weight, seed);
	}

	// 4. Take the inverse of the sample as a weight and multiply it by the average weight (color) of the bad samples
	// To compensate for the weight being more likely to be picked.
	float4 total_sample_color = float4(0.0f);
	float average_weight_samples = r1.w_sum / r1.m;
	float r1_norm_weight = average_weight_samples / r1.w;

	// r1
	x.direction = r1.y;
	float4 sample_color = float4(0.0f);
	transport(x, structure, scene, gid, bounces, sample_color, seed, true, normal, terminate_flag, true);
	x = r;
	
	total_sample_color += sample_color * r1_norm_weight;

	color *= total_sample_color;
	buffer.write(color, gid);
}

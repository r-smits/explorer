#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

// Math constants
constant float PI = 3.1415926535897932384626433832795;
constant float TAU = PI * 2;
constant float PI_INVERSE = 1 / PI;

// ReSTIR constants
constant int num_samples = 3;
constant float3 samples[samples]; 

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

struct Sample {
	
	float4

// Generates float within domain 0 <= n <= 1
float rand(thread uint32_t& seed) {
	seed = pcg_hash(seed);
	return (float)seed / (float)0xffffffff;
}

// Normalizes float between -1 <= n <= 1, then normalizes so vec3 sums to 1.
float3 random_float3(thread uint32_t& seed) {
	return normalize(
		float3(
			rand(seed) * 2 - 1, 
			rand(seed) * 2 - 1, 
			rand(seed) * 2 - 1
		)
	);
}

struct Reservoir {
	float w_sum;							// sum of weights
	float m;									// number of samples
	float y;									// chosen sample (ray direction)
	float w;									// weight

	void update(constant float3& sample, constant float& weight) {
		w_sum += weight;
		m += 1;
	}
};

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
float3 uniform_pdf(thread uint32_t& seed) {
	return normalize(
		float3(
			rand(seed), 
			rand(seed), 
			rand(seed)
		)
	);
}

float wi_dot_n_inverse_square(
	thread float& distance, 
	thread float3& wi, 
	thread float3& normal
) {
	float cosine = max(0.001, saturate(dot(wi, normal)));
	float result = cosine / abs(distance * distance);
	return result;
}

float visibility_check() {
	return 1;
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
	thread raytracing::intersector<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data>& intersector,
	thread raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data>& intersection, 
	constant Scene* scene,
	uint2 gid,
	thread int bounces,
	thread float4& color,
	thread uint32_t& seed,
	thread bool unshadowed_light_contribution
) {

	// Contribution is set to 1 and will decrease over time, with the amount of bounces
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 sky_color = float4(.2f, .3f, .4f, 1.0f);

	for (int i = 1; i <= bounces; i++) {
		// Verify if ray has intersected the geometry
		intersection = intersector.intersect(r, structure, 0xFF);

		// If our ray does not hit, we return sky color, e.g. black.
		if (intersection.type == raytracing::intersection_type::none) {
			//color *= contribution;
			color = (color + sky_color) + contribution; 
			break;
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
			float3 normal = (attr_1.normal * bary_3d.x) + (attr_2.normal * bary_3d.y) + (attr_3.normal * bary_3d.z);
			normal = normalize((intersection.object_to_world_transform * float4(normal, 0.0f)).xyz);
				
			thread uint32_t seed = gid.x * (tri_index_1 * bary_2d.x * i) + gid.y * (tri_index_3 * bary_2d.y * (bounces - i));
			r.origin = r.origin + r.direction * intersection.distance; 
			normal = normalize(normal + uniform_pdf(seed) * .6);
			r.direction = reflect(r.direction, normal);

			float3 wi = normalize(r.direction);
			float wi_dot_n = wi_dot_n_inverse_square(intersection.distance, wi, normal);

			// Calculate all color contributions; use texture / emission if there is one
			float2 tx_point = (attr_1.texture * bary_3d.x) + (attr_2.texture * bary_3d.y) + (attr_3.texture * bary_3d.z);
			float4 wo_color = (submesh.textured) ? texture.sample(sampler2d, tx_point) : attr_1.color;
				
			contribution *= wo_color * wi_dot_n;
			color *= (unshadowed_light_contribution) ? 1 : visibility_check();
			color += submesh.emissive * wo_color;
		}
	}
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
	
	// Check if instance acceleration structure was built succesfully
	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, gid);
		return;
	}
	
	// Initialize default parameters
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	thread uint32_t seed = 0;

	// Build ray. Ray shoots out from point (gid). Camera is a grid, point is a coordinate on the grid. 
	raytracing::ray r = build_ray(resolution, transform, gid);
	raytracing::ray x;
	
	// Build intersector. This object is responsible to check if the loaded instances were intersected.
	thread raytracing::intersector<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersector;	
	intersector.assume_geometry_type(raytracing::geometry_type::triangle);
	raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersection;

	// Shoot initial ray from the camera into the scene. 
	// This will set the ray, color and seed by reference. 
	transport(r, structure, intersector, intersection, scene, gid, 0, color, seed, true);
	x = r;
	
	// ReSTIR implementation

	// 1. Calculate the samples from the uniform pdf.
	// Take 3 random samples of the hemisphere around point x, this will be our uniform pdf.
	// All samples in a uniform have equal weights, so we don't have to calculate them. The sum of weights would be 1.
	for (int i = 0; i < num_samples; i += 1) samples[i] = uniform_pdf(seed);	
	
	// 2. Calculate the weights of the complex pdf through the unshadowed light contribution.
	// For the complex pdf, we take the color contribution as the weights. 
	// So color signifies both as the sample (output) of the funcion, as well as the weight attributed to that sample.
	for (int i = 0; i < num_samples; i += 1) {
		float4 weight = float4(1.0f);
		x.direction = samples[i];
		transport(x, structure, intersector, intersection, scene, gid, 3, weight, seed, true);

		// Before you re-assign ray x to y to choose a new random direction, you may need x.direction
		// Because there is a relation between the direction vector and the weight you calculated.
		// Because if the weight gets sampled, you have to use that direction to shoot the ray in to calculate the color.
		x = r;
	}

	// 3. Sample the complex pdf built during step 2 and calculate the color contribution. 
	// The first time, the output only counts as weights for the complex pdf.
	// You will then have built the complex pdf, and you need to sample it.
	
	// To build this complex pdf, you need to:
	//	A) Store the weights in some array with size M (total of bad samples drawn to turn into weights)
	//	B) Store the associated directions in some array with size M
	//	C) Create a reservoir abstraction.
	//	D) Call storage fn of reservoir, taking the weight, and sample (direction).
	//	E) Add weight to total weight. Increment the total samples seen.
	//	E) In fn, flip a weighted coin. If heads, keep the weight and sample. If tails, discard them. 
	//		 The coin is weighted. The weight of the incoming is heads. The weight of the outgoing is tails.
	//	F) Return the sample left in the reservoir. Use the direction to init the ray and start transport fn.
	//	G) 

	// 4. Take the inverse of the sample as a weight and multiply it by the average weight (color) of the bad samples
	// The bad samples are the samples calculated during step 2 (I think).
	// To compensate for the weight being more likely to be picked.
	// (average_weight_of_bad_samples) / (weight_of_good_sample) 

	// Then, you need to compensate for the fact that you're using a uniform pdf to emulate sampling from a complex pdf
	// You do this by taking the average of the sum of the weights of the bad samples



	// You build the PDF by assigning a weight, picking a number and returning a color based on that number.
	// In other words, you need to use reservoir sampling for this.

	//thread float4 sample_colors[samples];

	//for (int i = 0; i < samples; i+=1) {
	//	float4 sample_color = color;
	//	x.direction = uniform_pdf(seed) * .7);
	//	transport(x, structure, intersector, intersection, scene, gid, 3, sample_color, seed);
	//	
	// Save the variables set by reference
	//	sample_colors[i] = sample_color;
	//	x = r;
	//}
	
	// Calculate light contribution. Ray, intersection, color are set by reference.
	// transport(r, structure, intersector, intersection, scene, gid, 10, color, seed);

	// Write color to texture.
	buffer.write(color, gid);
}
	

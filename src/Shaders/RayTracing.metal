#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

constexpr sampler sampler2d(address::clamp_to_edge, filter::linear); 

constant float PI = 3.1415926535897932384626433832795;
constant float TAU = PI * 2;
constant float PI_INVERSE = 1 / PI;

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

uint32_t pcg_hash(thread uint32_t input) {
	uint32_t state = input * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

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
	float y;									// chosen sample
	float w_sum;							// sum of weights
	float m;									// number of samples
	float w;									// weight

	void update(constant float3& sample, constant float& weight) {
		w_sum += weight;
		m += 1;
	}
};

float3 bitangent(float3 u)
{
	float3 a = abs(u);
	uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
	uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
	uint zm = 1 ^ (xm | ym);
	return cross(u, float3(xm, ym, zm));
}

float wi_dot_n(float distance, float3 wi, float3 normal) {
	float cosine = max(0.001, saturate(dot(wi, normal)));
	float result = cosine / abs(distance * distance);
	return result;
}

raytracing::ray buildRay(
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
		// Initialize default color
		float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
		float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
		
		// Check if instance acceleration structure was built succesfully
		if (is_null_instance_acceleration_structure(structure)) {
			buffer.write(color, gid);
			return;
		}

		// Ray shoots out from point (gid). 
		// Camera is a grid, point is a coordinate on the grid. 
		raytracing::ray r = buildRay(resolution, transform, gid);
		
		// Build intersector. This object is responsible to check if the loaded instances were intersected.
		raytracing::intersector<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersector;	
		intersector.assume_geometry_type(raytracing::geometry_type::triangle);
	
		// The amount of times we allow for the ray to bounce from object to object.
		int bounces = 2;
		raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data, raytracing::world_space_data> intersection;
		
		for (int i = 1; i <= bounces; i++) {
			// Verify if ray has intersected the geometry
			intersection = intersector.intersect(r, structure, 0xFF);

			// If our ray does not hit, we return sky color, e.g. black.
			if (intersection.type == raytracing::intersection_type::none) { break; } 
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
				float3 hit = r.origin + r.direction * intersection.distance;
				r.origin = hit + normal * 0.001;
				normal = normalize(normal + random_float3(seed) * .25);
				r.direction = reflect(r.direction, normal);

				float3 wi = normalize(r.direction);
				float wi_dot_n = max(0.001, saturate(dot(wi, normal)));
				
				// Calculate all color contributions; use texture / emission if there is one
				float2 tx_point = (attr_1.texture * bary_3d.x) + (attr_2.texture * bary_3d.y) + (attr_3.texture * bary_3d.z);
				float4 wo_color = (submesh.textured) ? texture.sample(sampler2d, tx_point) : attr_1.color;
				
				contribution *= wo_color * wi_dot_n;
				color += submesh.emissive * wo_color * 1;
			}
		}
		color += contribution * float4(.2f, .3f, .4f, 1.0f);
		buffer.write(color, gid);
	}
	

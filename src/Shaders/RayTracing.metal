#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

constexpr sampler sampler2d(address::clamp_to_edge, filter::linear); 

constant float PI = 3.1415926535897932384626433832795;
constant float TAU = PI * 2;
constant float PI_INVERSE = 1 / PI;

struct VertexAttributes {
	float3 color;																	// {r, g, b}
	float2 texture;																// {x, y}
	float3 normal;																// v{x, y, z}
};

struct Submesh
{
    constant uint32_t* indices;									// Indices pointing at the packed vertices
		texture2d<float> texture;
};

struct Mesh
{
		constant packed_float3* vertices;						// Vertices packed: XYZXYZ...
    constant VertexAttributes* attributes;			// Attributes of the vertices
		constant Submesh* submeshes;								// Submeshes related to the mesh
};

struct Scene
{
		constant Mesh* meshes;											// All meshes related to all models
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

struct Reservoir {
	float y;								// chosen sample
	float wsum;							// sum of weights
	float M;								// number of 'bad' samples
	float W;								// weight
};


/**
void generateSamples(float2 p) {

	Reservoir reservoir = {0.0f, 0.0f, 0.0f, 0.0f};

	for (int i = 0; i < 32; i++) {
		int lightToSample = min(int(rng(p) * 	



	}



}
**/


float rng(float2 p) {
  float2 r = float2(23.1406926327792690, 2.6651441426902251);	
	// e^pi (Gelfond's constant), 2^sqrt(2) (Gelfond–Schneider constant)
	return fract(cos(fmod(123456789., 1e-7 + 256. * dot(p,r))));  
}

float3 bitangent(float3 u)
{
	float3 a = abs(u);
	uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
	uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
	uint zm = 1 ^ (xm | ym);
	return cross(u, float3(xm, ym, zm));
}

float3 hemiSample(float2 random, float3 normal) {
	float3 bitan = bitangent(normal);
  float3 tan = cross(bitan, normal);
  float radius = sqrt(random.x);
	float phi = TAU * random.y;
  return tan		 * (radius * cos(phi)) + bitan 
								 * (radius * sin(phi)) + normal 
								 * sqrt(max(0.0f, 1.0f - random.x));
};

raytracing::ray buildRay(
	constant float3& resolution,
	constant RTTransform &transform,
	uint2 gid
) {
	// Pixel coordinate needs to be within world space:
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
	float3 rayDirection = (transform.mView * projTransformN).xyz;

	raytracing::ray r;
	r.origin = transform.rayOrigin;
	r.direction = rayDirection;
	r.min_distance = 0.1f;
	r.max_distance = FLT_MAX;
	return r;
}

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer									[[ texture(0)								]],
	constant float3& resolution															[[ buffer(0)								]],
	constant float3& lightDir																[[ buffer(1)								]],
	constant RTTransform& transform													[[ buffer(2)								]],
	raytracing::instance_acceleration_structure structure		[[ buffer(3)								]],
	constant Scene* scene																		[[ buffer(4)								]],
	constant float4x4& modelMatrix													[[ buffer(20)								]],
	uint2 gid																								[[ thread_position_in_grid	]] 
) {
		// Initialize default color
		float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		
		// Check if instance acceleration structure was built succesfully
		if (is_null_instance_acceleration_structure(structure)) {
			buffer.write(color, gid);
			return;
		}

		// Build initial ray shoots out from the location of the pixel. Accounts for cameraView matrix.
		raytracing::ray r = buildRay(resolution, transform, gid);
		
		float2 pixel = float2(gid);
		pixel /= resolution.xy;

		// Build intersector. This object is responsible to check if the loaded instances were intersected.
		raytracing::intersector<raytracing::instancing, raytracing::triangle_data> intersector;
		intersector.assume_geometry_type(raytracing::geometry_type::triangle);
	
		// The amount of times we allow for the ray to bounce from object to object.
		int bounces = 2;
		float factor = 1.0f;
		raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data> intersection;
		for (int i = 0; i < bounces; i++) {
			
			// Verify if ray has intersected the geometry
			intersection = intersector.intersect(r, structure, 0xFF);
			
			// If our ray does not hit, then we make the color of the sky light up a little
			if (intersection.type == raytracing::intersection_type::none) {
				float3 skyColor = float3(0.01f, 0.1f, 0.1f);
				color.xyz * skyColor * factor;
				break;
			}

			if (intersection.type == raytracing::intersection_type::triangle) {
			
				// Look up the data belonging to the intersection in the scene
				// This requires a bindless setup
				Mesh mesh = scene->meshes[intersection.instance_id];
				Submesh submesh = mesh.submeshes[intersection.geometry_id];
				texture2d<float> texture = submesh.texture;

				float2 bary2 = intersection.triangle_barycentric_coord;
				float3 bary3 = float3(1.0 - bary2.x - bary2.y, bary2.x, bary2.y);

				uint32_t index1 = submesh.indices[intersection.primitive_id * 3 + 0];
				uint32_t index2 = submesh.indices[intersection.primitive_id * 3 + 1];
				uint32_t index3 = submesh.indices[intersection.primitive_id * 3 + 2];

				float3 pos1 = (modelMatrix * float4(mesh.vertices[index1], 1.0f)).xyz;
				float3 pos2 = (modelMatrix * float4(mesh.vertices[index2], 1.0f)).xyz;
				float3 pos3 = (modelMatrix * float4(mesh.vertices[index3], 1.0f)).xyz;

				float3 u = pos2 - pos1;
				float3 v = pos3 - pos1;




				float3 normal1 = mesh.attributes[index1].normal;
				float3 normal2 = mesh.attributes[index2].normal;
				float3 normal3 = mesh.attributes[index3].normal;

				float2 txCoord1 = mesh.attributes[index1].texture;
				float2 txCoord2 = mesh.attributes[index2].texture;
				float2 txCoord3 = mesh.attributes[index3].texture;
				
				float3 pos = (pos1 * bary3.x) + (pos2 * bary3.y) + (pos3 * bary3.z);
				float3 normal = normalize((normal1 * bary3.x) + (normal2 * bary3.y) + (normal3 * bary3.z));
				normal = (modelMatrix * float4(normal, 1.0f)).xyz;

				//normal = normalize(cross(u, v));

				

				float2 txCoord = (txCoord1 * bary3.x) + (txCoord2 * bary3.y) + (txCoord3 * bary3.z);

				// We now know our ray hits. We can continue to calculate the reflection.
				float3 objectColor = texture.sample(sampler2d, txCoord).xyz;
								
				// Lambertian BSDF: bi-drectional scattering distribution function
				// diffuse lighting uniformly relative to the incoming light direction (wi).
				float3 wi = -normalize(lightDir);
				float lambertianBSDF = saturate(dot(normal, wi)) * factor;
				color.xyz = color.xyz * objectColor * lambertianBSDF;

				//float3 pos = r.origin + r.direction * intersection.distance;

				//color.xyz = r.origin + r.direction * intersection.distance;
				//color.xyz = normal;
				
				// reSTIR GI
				// 1) Generate sample for visible point pos
				//float2 random2 = float2(rng(pixel), rng(pixel));
				//float3 L = normalize(hemiSample(random2, normal));



				
				// We assume for now that the surface is perfectly reflective.
				r.origin = pos + normal * 0.001;
				r.direction = reflect(r.direction, normal + 0.0 * (rng(pixel) -0.5f)); 

			}
		}

		buffer.write(color, gid);
	}
	

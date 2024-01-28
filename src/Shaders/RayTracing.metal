#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

constexpr sampler sampler2d(address::repeat, filter::linear);

/**
constexpr sampler linearSampler (address::repeat,
                                 mip_filter::linear,
                                 mag_filter::linear,
                                 min_filter::linear);
**/


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

struct Model
{
		constant Mesh* meshes;											// Meshes related to model
};

struct Scene
{
    constant Model* models;											// All models in the scene
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

struct Sphere {
	float3 origin;
	float radius;
};

struct Payload {
	int index;
	float distance;
	float3 position;
	float3 normal;
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

Payload hit(
	raytracing::ray r,
	constant Sphere* spheres,
	constant float& count
) {
	int index = -1;
	float distance = 9999;
	for (int i = 0; i < count; ++i) {

		float3 rayOrigin = r.origin - spheres[i].origin;
		float radius = spheres[i].radius;

		// Intersection logic
		// ray origin =				a
		// ray direction =		b
		// radius =						r
		// distance =					d
	
		float a = dot(r.direction, r.direction);
		float b = 2.0f * dot(rayOrigin, r.direction);
		float c = dot(rayOrigin, rayOrigin) - pow(radius, 2);
	
		// b^2 -4 * ac
		float discriminant = b * b - 4.0f * a * c;
	
		if (discriminant <= 0.0f) continue;
		
		float intersect = (-b - sqrt(discriminant)) / (2.0f * a); // Closest intersection point

		if (intersect > 0.0f && intersect < distance) {
			distance = intersect;
			index = i;
		}
	}

	float3 origin = r.origin - spheres[index].origin;
	float3 point = origin + r.direction * distance;
	float3 normal = normalize(point);
	point += spheres[index].origin;

	Payload p;
	p.index = index;
	p.distance = distance;
	p.position = point;
	p.normal = normal;
	return p;
}

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer								[[texture(0)]],
	constant float3& resolution														[[buffer(0)]],
	constant float3& lightDir															[[buffer(1)]],
	constant RTTransform& transform												[[buffer(2)]],
	constant RTMaterial* materials												[[buffer(4)]],
	raytracing::instance_acceleration_structure structure [[buffer(6)]],
	constant Scene* scene																	[[buffer(9)]],
	uint2 gid																							[[thread_position_in_grid]] 
) {
		// Initialize default color
		float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
		
		// Check if instance acceleration structure was built succesfully
		if (is_null_instance_acceleration_structure(structure)) {
			buffer.write(color, gid);	
		}

		// Build initial ray shoots out from the location of the pixel. Accounts for cameraView matrix.
		raytracing::ray r = buildRay(resolution, transform, gid);

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
				float3 skyColor = float3(0.0f, 0.0f, 0.0f);
				color.xyz += skyColor * factor;
				break;
			}

			if (intersection.type == raytracing::intersection_type::triangle) {
			
				// Look up the data belonging to the intersection in the scene
				// This requires a bindless setup
				Mesh mesh = scene->models[intersection.instance_id].meshes[0];
				Submesh submesh = mesh.submeshes[intersection.geometry_id];
				texture2d<float> texture = submesh.texture;

				float2 bary2 = intersection.triangle_barycentric_coord;
				float3 bary3 = float3(1.0 - (bary2.x + bary2.y), bary2.x, bary2.y);

				uint32_t index1 = submesh.indices[intersection.primitive_id * 3 + 0];
				uint32_t index2 = submesh.indices[intersection.primitive_id * 3 + 1];
				uint32_t index3 = submesh.indices[intersection.primitive_id * 3 + 2];

				float3 pos1 = mesh.vertices[index1];
				float3 pos2 = mesh.vertices[index2];
				float3 pos3 = mesh.vertices[index3];

				float3 normal1 = mesh.attributes[index1].normal;
				float3 normal2 = mesh.attributes[index2].normal;
				float3 normal3 = mesh.attributes[index3].normal;

				float2 txCoord1 = mesh.attributes[index1].texture;
				float2 txCoord2 = mesh.attributes[index2].texture;
				float2 txCoord3 = mesh.attributes[index3].texture;
				
				float3 pos = (pos1 * bary3.x) + (pos2 * bary3.y) + (pos3 * bary3.z);
				float3 normal = (normal1 * bary3.x) + (normal2 * bary3.y) + (normal3 * bary3.z);
				float2 txCoord = (txCoord1 * bary3.x) + (txCoord2 * bary3.y) + (txCoord3 * bary3.z);

				// We now know our ray hits. We can continue to calculate the reflection.
				float3 objectColor = texture.sample(sampler2d, txCoord).xyz;
				float3 lightDirN = normalize(lightDir);

				// Angle between the outgoing light vector and normal, 90 degrees to the point
				// We assume for now that the surface is perfectly reflective.
				float cosTheta = max(dot(normal, -lightDirN), 0.0f);
				objectColor *= cosTheta;
				color.xyz += objectColor * factor;
				factor *= 0.7f;

				//color = float4(objectColor, 1.0f);


				//float3 origin = r.origin - spheres[index].origin;
				//float3 point = origin + r.direction * distance;

				r.origin = pos + normal * 0.001;
				r.direction = reflect(r.direction, normal);
			}
		}

		buffer.write(color, gid);
	}
	

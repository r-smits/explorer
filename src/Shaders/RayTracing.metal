#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

constexpr sampler kernelsampler2d(address::clamp_to_edge, filter::linear);

struct VertexAttributes {
	float3 color;																	// {r, g, b}
	float2 texture;																// {x, y}
	float3 normal;																// v{x, y, z}
};

struct Submesh
{
    constant uint32_t* indices;									// Indices pointing at the packed vertices
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
	constant Sphere* spheres															[[buffer(3)]],
	constant RTMaterial* materials												[[buffer(4)]],
	constant float& spherecount														[[buffer(5)]],
	raytracing::instance_acceleration_structure structure [[buffer(6)]],
	constant Scene* scene																	[[buffer(9)]],
	uint2 gid																							[[thread_position_in_grid]] 
) {
		// Initialize default color
		float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
		
		// Verifying bindless scene works
		if (scene->models[0].meshes[0].submeshes[0].indices[1083] == 333) {
			if (scene->models[0].meshes[0].submeshes[1].indices[5] == 3274) {
			color = float4(0.0f, 1.0f, 0.2f, 1.0f);
			}
		}
		
		// Check if instance acceleration structure was built succesfully
		if (is_null_instance_acceleration_structure(structure)) {
			buffer.write(color, gid);	
		}

		// Build initial ray shoots out from the location of the pixel.
		// It is transformed to account for the cameraView matrix.
		raytracing::ray r = buildRay(resolution, transform, gid);

		// Build intersector. This object is responsible to check if the loaded instances were intersected.
		// We are working with triangles.
		raytracing::intersector<raytracing::instancing, raytracing::triangle_data> intersector;
		intersector.assume_geometry_type(raytracing::geometry_type::triangle);
	
		// The amount of times we allow for the ray to bounce from object to object.
		int bounces = 1;
		float factor = 1.0f;
		raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data> intersection;
		for (int i = 0; i < bounces; i++) {
			
			// Verify if ray has intersected the geometry
			intersection = intersector.intersect(r, structure, 0xFF);
			
			// If our ray does not hit, then we make the color of the sky light up a little
			// Right now the skycolor is black, so it's not visible
			// The factor would be weaker the more often the ray has bounced before reaching this point
			if (intersection.type == raytracing::intersection_type::none) {
				float3 skyColor = float3(0.0f, 0.0f, 0.0f);
				color.xyz += skyColor * factor;
				break;
			}
			
			// We now know our ray hits. We can continue to calculate the reflection.
			float3 objectColor = float3(0.0f, 0.0f, 1.0f);
			float3 lightDirN = normalize(lightDir);
			
			color.xyz = objectColor;
			factor *= 0.7f;

			// WIP
			// Angle between the outgoing light vector and normal, 90 degrees to the point
			// We assume for now that the surface is perfectly reflective.

			// How do we calculate the normal?
			// We have the normal. It should be located in the first buffer of the mesh.
			// So, you need your models as input to this function.

			// You need to know where the triangle was intersected.
			// After you have that point, you need to find the matching normal to that point.
			// You would need to find the index of the vertex in the indexbuffer.
			// Then, you would need to find that in the vertex buffer.


			// float cosTheta = max(dot(h.normal, -lightDirN), 0.0f);
			// objectColor *= cosTheta;
			// color.xyz += objectColor * factor;
			// factor *= 0.7f;
			
			// WIP
			// r.origin = h.position + h.normal * 0.001;
			// r.direction = reflect(r.direction, h.normal);

		}

		buffer.write(color, gid);
	}
	
	/**
	raytracing::ray r = buildRay(resolution, transform, gid);
	raytracing::intersector<raytracing::instancing, raytracing::triangle_data> intersector;
  intersector.assume_geometry_type(raytracing::geometry_type::triangle); 
	raytracing::intersection_result<raytracing::instancing, raytracing::triangle_data> intersection = intersector.intersect(r, structure, 0xFF);
	**/

	// intersection_result contains the following information:
	// type -> type of intersection. ::none if not intersected, ::triangle if hit.
	// instance_id -> the id of the instance which was hit.
	// triangle_front_facing -> if the front of the triangle was intersected, then it will be true.
	// barymetric_coordinate -> ...
	
	/**
	if (intersection.type == raytracing::intersection_type::triangle) {
		color = float4(1.0f, 0.0f, 0.0f, 1.0f);

		if (intersection.instance_id == 0) {
			color = float4(0.0f, 0.0f, 1.0f, 1.0f);
		}

		if (intersection.triangle_front_facing == true) {
			color = float4(1.0f, 0.0f, 1.0f, 1.0f);
		}
	}

	if (intersection.type == raytracing::intersection_type::none) {
		color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	**/
		
	// buffer.write(color, gid);





	/**
	int bounces = 2;

	float factor = 1.0f;
	for (int i = 0; i < bounces; i++) {

		//auto intersection = intersector.intersect(r, structure, 0xFF);

		Payload h = hit(r, spheres, spherecount);

		if (h.index == -1) {
			float3 skyColor = float3(0.0f, 0.0f, 0.0f);
			color.xyz += skyColor * factor;
			break;
		}
	
		float3 sphereColor = materials[h.index].color;
		float3 lightDirN = normalize(lightDir);

		// Angle between the outgoing light vector and normal, 90 degrees to the point
		float cosTheta = max(dot(h.normal, -lightDirN), 0.0f);
		sphereColor.xyz *= cosTheta;
		color.xyz += sphereColor * factor;
		factor *= 0.7f;

		r.origin = h.position + h.normal * 0.001;
		r.direction = reflect(r.direction, h.normal);
	}

	buffer.write(color, gid);
	**/
//}


[[kernel]]
void colorCheck(
	texture2d<float, access::write> buffer [[texture(0)]],
	const device float4& resolution [[buffer(0)]],
	const device float3& lightDir [[buffer(1)]],
	const device RTTransform& transform [[buffer(2)]],
	uint2 gid [[thread_position_in_grid]] 
) {

	float2 color = float2(gid) / resolution.xy;
	buffer.write(float4(color.x, color.y, 0.0, 1.0), gid);
}



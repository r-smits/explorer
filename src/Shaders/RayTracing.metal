#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;

constexpr sampler kernelsampler2d(address::clamp_to_edge, filter::linear);

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
	r.min_distance = 0.1;
  r.max_distance = FLT_MAX;

	return r;
}

Payload hit(
	raytracing::ray r,
	constant Sphere* spheres,
	constant float& count
) {
	int index = -1;
	float distance = FLT_MAX;
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
	texture2d<float, access::write> buffer [[texture(0)]],
	constant float3& resolution [[buffer(0)]],
	constant float3& lightDir [[buffer(1)]],
	constant RTTransform& transform [[buffer(2)]],
	constant Sphere* spheres [[buffer(3)]],
	constant RTMaterial* materials [[buffer(4)]],
	constant float& spherecount [[buffer(5)]],
	uint2 gid [[thread_position_in_grid]] 
) {
	raytracing::ray r = buildRay(resolution, transform, gid); 
		
	int bounces = 2;

	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float factor = 1.0f;
	for (int i = 0; i < bounces; i++) {
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
}


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



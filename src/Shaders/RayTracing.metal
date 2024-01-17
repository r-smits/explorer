#include <metal_stdlib>
using namespace metal;

constexpr sampler kernelsampler2d(address::clamp_to_edge, filter::linear);

struct RTTransform {
	simd::float4x4 mProjection;
	simd::float4x4 mView;
  simd::float4x4 mInverseProjection;
  simd::float4x4 mInverseView;
	simd::float3 rayOrigin;
};


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

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer [[texture(0)]],
	const device float3& resolution [[buffer(0)]],
	const device float3& lightDir [[buffer(1)]],
	const device RTTransform& transform [[buffer(2)]],
	uint2 gid [[thread_position_in_grid]] 
) {
	// Pixel coordinate needs to be within world space:
	// -1 >= x >= 1 :: Normalized
	// -1 >= y >= 1 :: We want to scale y in opposite direction for viewport coordinates
	// -1 >= z >= 0 :: The camera is pointed towards -z axis, as we use right handed
	float3 pixel = float3(float2(gid), 1.0f);
	pixel.x /= resolution.x;
	pixel.y /= resolution.y;
	pixel = pixel * 2 - 1;
	pixel *= float3(1, -1, 1);

	// Projection transformations
	float4 projTransform = transform.mInverseProjection * float4(pixel, 1);
	float4 projTransformN = float4(normalize(projTransform.xyz / projTransform.w), 0.0f);
	float3 rayDirection = (transform.mView * projTransformN).xyz;

	//float4 viewTransform = transform.mInverseView * float4(pixel, 1);
	//float3 viewTransformN = normalize(viewTransform.xyz / viewTransform.w);
	//float4 projTransform = transform.mInverseProjection * float4(viewTransformN, 1);
	//float3 rayDirection = normalize(projTransform).xyz;

	//float3 rayDirection = normalize(pixel);
	
	float3 rayOrigin = transform.rayOrigin;
	float radius = 0.5f;

	// Intersection logic
	// ray origin =				a
	// ray direction =		b
	// radius =						r
	// distance =					d
	
	float a = dot(rayDirection, rayDirection);
	float b = 2.0f * dot(rayOrigin, rayDirection);
	float c = dot(rayOrigin, rayOrigin) - pow(radius, 2);
	
	// b^2 -4 * ac
	float discriminant = b * b - 4.0f * a * c;
	
	if (discriminant <= 0.0f) {
		buffer.write(float4(0.0f, 0.0f, 0.0f, 1.0f), gid);
		return;
	}

	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);

	float intersect = (-b - sqrt(discriminant)) / (2.0f * a); // Closest intersection point

	float3 point = rayOrigin + rayDirection * intersect;
	
	float3 normal = normalize(point);

	float3 lightDirN = normalize(lightDir);

	// Angle between the outgoing light vector and normal, 90 degrees to the point
	float cosTheta = max(dot(normal, -lightDirN), 0.0f); 

	color.xyz = clamp((color.xyz *= cosTheta), 0.0f, 1.0f);

	buffer.write(color, gid);
	//buffer.write(float4(normal, 1), gid);
}


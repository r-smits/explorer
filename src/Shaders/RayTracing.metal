#include <metal_stdlib>
using namespace metal;

constexpr sampler kernelsampler2d(address::clamp_to_edge, filter::linear); 

[[kernel]]
void colorCheck(
	texture2d<float, access::write> buffer [[texture(0)]],
	const device float4& resolution [[buffer(0)]],
	const device float3& lightDir [[buffer(1)]],
	uint2 gid [[thread_position_in_grid]] 
) {

	float2 color = float2(gid) / resolution.xy;
	buffer.write(float4(color.x, color.y, 0.0, 1.0), gid);
}

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer [[texture(0)]],
	const device float4& resolution [[buffer(0)]],
	const device float3& lightDir [[buffer(1)]],
	uint2 gid [[thread_position_in_grid]] 
) {

	// Pixel coordinate needs to be within world space:
	// -1 >= x >= 1
	// -1 >= y >= 1 // We want to scale y in opposite direction for viewport coordinates
	//  0 >= z >= 1
	float3 direction = float3(float2(gid), 1.0f);
	direction.xy /= resolution.xy;
	direction = direction * 2 - 1;
	direction.y *= -1;

	// ray origin =				a
	// ray direction =		b
	// radius =						r
	// distance =					d
	float3 origin = float3(0.0f, 0.0f, -1.0f);
	float radius = 0.5f;

	float a = dot(direction, direction);
	float b = 2.0f * dot(origin, direction);
	float c = dot(origin, origin) - pow(radius, 2);

	float discriminant = b * b - 4.0f * a * c;
	
	if (discriminant <= 0.0f) {
		buffer.write(float4(0.0f, 0.0f, 0.0f, 1.0f), gid);
		return;
	}

	float4 color = float4(0.0f, 0.0f, 1.0f, 1.0f);

	//float intersect1 = (-b + sqrt(discriminant)) / (2.0f * a);
	float intersect2 = (-b - sqrt(discriminant)) / (2.0f * a); // Closest intersection point

	float3 point2 = origin + (direction * intersect2);

	float3 normal2 = normalize(point2);

	float3 lightDirN = normalize(lightDir);
	float cosTheta2 = max(dot(normal2, -lightDirN), 0.0f);

	color.xyz *= cosTheta2;

	buffer.write(color, gid);
}


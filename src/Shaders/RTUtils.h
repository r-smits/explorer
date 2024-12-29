#pragma once

#ifndef RTUtils_h
#define RTUtils_h

#if __METAL_VERSION__

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
			rand(seed), 
			rand(seed), 
			rand(seed)
		) * 2 - 1
	);
}

float3 vec_perpendicular(float3 u) {
	float3 a = abs(u);
	uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
	uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
	uint zm = 1 ^ (xm | ym);
	return cross(u, float3(xm, ym, zm));
}


// Get a uniform weighted random vector centered around a specified normal direction.
float3 rand_hemisphere(thread uint32_t& seed, float3 normal) {
	float2 point_rand = float2(rand(seed), rand(seed));

	float3 bitangent = vec_perpendicular(normal);
	float3 tangent = cross(bitangent, normal);
	float r = sqrt(max(0.0f, 1.0f - point_rand.x * point_rand.x));
	float phi = 2.0f * 3.14159265f * point_rand.y;

	return tangent * (r * cos(phi)) + bitangent * (r * sin(phi)) + normal.xyz * point_rand.x;
}


// Cosine: decrease light intensity based on the angle between the normal and outgoing light direction (wi).
float lambertian(
	thread float3& wi,
	thread float3& normal
) {
	return max(0.001, saturate(dot(normalize(wi), normal)));
}

// Inverse square: light intensity inversely proportional to distance.
float cos_inverse_square(
	thread float& distance, 
	thread float3& wi, 
	thread float3& normal
) {
	float cosine = lambertian(wi, normal);
	float result = cosine / abs(distance * distance);
	return result;
}

float4 compare(float3 vec1, float3 vec2) {
	if (distance(vec1, vec2) < 0.01) {
		return float4(.0f, 1.0f, .0f, .0f);	
	} else {
		return float4(1.0f, .0f, .0f, .0f);
	}
}

#endif
#endif


#pragma once

#ifndef RTUtils_h
#define RTUtils_h

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

void build_ray(thread ray& r, constant VCamera& vcamera, uint2 gid) {
	
	// Camera point needs to be within world space:
	// -1 >= x >= 1 :: Normalized
	// -1 >= y >= 1 :: We want to scale y in opposite direction for viewport coordinates
	// -1 >= z >= 0 :: The camera is pointed towards -z axis, as we use right handed
	float3 pixel = float3(float2(gid), 1.0f);
	pixel *= float3(1, -1, 1);
	// ??? Somehow works without this line
	// pixel = pixel / resolution * 2 - 1;

	// Projection transformations
	// orientation = matView * matProjection
	float3 vecRayDir = (orientation * float4(pixel, 1.0f)).xyz;
	
	// Ray is modified by reference
	r.origin = vecOrigin;
	r.direction = vecRayDir;
	r.min_distance = 0.2f;						// Set to avoid self-occlusion
	r.max_distance = FLT_MAX;
}

#endif

#pragma once

#ifndef ShaderTypes_h
#define ShaderTypes_h

#if __METAL_VERSION__

#import "RTUtils.h"

constexpr sampler sampler2d(address::clamp_to_edge, filter::linear);


struct RTMaterial {
	float3 color;
	float3 roughness;
	float3 metallic;
};


struct VCamera {
	float4x4 orientation;
	float3 vecOrigin;
	float3 resolution;
};


struct PrimitiveAttributes {
	float4 color[3];
	float2 txcoord[3];
	float3 normal[3];
	uint2 flags;
};


struct VertexAttributes {
	float4 color;															// {r, g, b, w}
	float2 texture;														// {x, y}
	float3 normal;														// v{x, y, z}
};


struct Submesh
{
  constant uint32_t* indices;								// Indices pointing at the packed vertices
	texture2d<float, access::sample> texture;
	bool textured;
	bool emissive;
};


struct Mesh
{
	constant packed_float3* vertices;					// Vertices packed: XYZXYZ...
  constant VertexAttributes* attributes;		// Attributes of the vertices
	constant Submesh* submeshes;							// Submeshes related to the mesh
	float4x4 orientation;
	int vertexCount;
};


struct Text2DSample {
	texture2d<float, access::sample> value;
};


struct Text2DReadWrite {
	texture2d<float, access::read_write> value;
};


struct Scene {
	constant Text2DSample* textsample;
	constant Text2DReadWrite* textreadwrite;
	constant VCamera* vcamera;
	constant Mesh* lights;
	uint8_t lightsCount;
};


struct GBufferIds {
	static constant uint8_t pos = 0;
	static constant uint8_t norm = 1;
	static constant uint8_t col = 2;
};


struct RestirIdx {
	static constant uint8_t prev_frame = 0;
};


struct PrimFlagIds {
	static constant uint8_t textid = 0;
	static constant uint8_t emissive = 1;
};


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


#endif
#endif


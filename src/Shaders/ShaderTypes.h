#pragma once

#ifndef ShaderTypes_h
#define ShaderTypes_h

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
	float txindex;
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
};

struct Scene
{
	constant Mesh* meshes;										// All meshes related to all models
};

#endif


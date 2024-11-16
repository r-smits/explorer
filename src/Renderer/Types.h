#pragma once
#include "Metal/MTLTypes.hpp"
#include <Renderer/Layout.h>
#include <pch.h>
#include <simd/simd.h>
#include <simd/vector_types.h>

struct Projection {
  simd::float4x4 model;
  simd::float4x4 camera;
};

namespace Renderer {

struct RTTransform {
  simd::float4x4 mProjection;
  simd::float4x4 mView;
  simd::float4x4 mInverseProjection;
  simd::float4x4 mInverseView;
  simd::float3 rayOrigin;
};

struct RTMaterial {
  simd::float3 color;
  simd::float3 roughness;
  simd::float3 metallic;
};

struct Material {
  simd::float4 color;    // {x, y, z, w}
  simd::float3 ambient;  // {x, y, z}
  simd::float3 diffuse;  // {x, y, z}
  simd::float4 specular; // {x, y, z, shininess}
  bool useColor;
  bool useLight;
  // simd::float2 texture;
};

struct Sphere {
  simd::float3 origin;
  float radius;
};

struct Vertex {
  simd::float3 position;
  simd::float3 color;
  simd::float2 texture;
  simd::float3 normal;
};

struct Light {
  simd::float3 position; // {x, y, z}
  simd::float3 color;    // {r, g, b}
  simd::float4 factors;  // {brightness, ambientIntensity, diffuseIntensity,
                         // specularIntensity}
};

struct Projection {
  simd::float4x4 camera;
  simd::float4x4 model;
  simd::float3 cameraPosition;
};

struct VertexAttributes {
  simd::float4 color;   // [[ id(0) ]]; // {r, g, b, w}
  simd::float2 texture; // [[ id(1) ]]; // {x, y}
  simd::float3 normal;  // [[ id(2) ]]; // {x, y, z}
};

struct Submesh {
  uint64_t indices;					// Indices pointing at the packed vertices
	MTL::ResourceID texture;	// Resource ID pointing at texture of submesh
	bool textured;						// Whether to use texture for indices
	bool emissive;						// Whether material is a light or not
};

struct Mesh {
  uint64_t vertices;						// Vertices packed: XYZXYZ...
  uint64_t attributes;					// Attributes of the vertices. See shader for data types
  uint64_t submeshes;						// Submeshes related to the mesh
};

//struct Model {
//  uint64_t meshes; // Meshes related to model
//};

struct Scene {
  uint64_t meshes; // All models in the scene
};

}; // namespace Renderer

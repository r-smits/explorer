#pragma once
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
  simd::float4 position;
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

}; // namespace Renderer

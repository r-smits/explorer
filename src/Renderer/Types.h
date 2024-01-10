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

struct Material {
	simd::float4 color;
	simd::float3 ambient;
	simd::float3 diffuse;
	simd::float3 specular;
	bool useColor;
	//simd::float2 texture;
	//simd::float1 shininess;
};

struct Vertex {
  simd::float4 position;
  simd::float3 color;
  simd::float2 texture;
	simd::float3 normal;
};

struct Light {
  simd::float3 position;		// {x, y, z}
	simd::float3 color;				// {r, g, b}
	simd::float4 factors;			// {brightness, ambientIntensity, diffuseIntensity, specularIntensity}
	//float ambientIntensity;
	//float diffuseIntensity;
	//float specularIntensity;
};

struct Projection {
	simd::float4x4 camera;
	simd::float4x4 model;
};

}; // namespace Renderer

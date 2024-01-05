#pragma once
#include <pch.h>
#include <Renderer/Layout.h>
#include <simd/simd.h>

namespace Renderer {

struct Vertex {
  simd::float4 position;
  simd::float3 color;
	simd::float2 texture;
};

struct Light {
  simd::float4 position;
};

struct Projection {
  simd::float4x4 projection;
};

}; // namespace Explorer

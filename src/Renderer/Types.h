#pragma once
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {

struct Vertex {
  simd::float4 position;
  simd::float3 color;
};

struct Light {
  simd::float4 position;
};

struct Projection {
  simd::float4x4 projection;
};
}; // namespace Explorer

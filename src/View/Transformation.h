#pragma once
#include <pch.h>

namespace Transformation {

simd::float4x4 identity();
simd::float4x4 translation(simd::float3);
simd::float4x4 zRotation(float theta);
simd::float4x4 scale(float factor);
// simd::float4x4 xRotation();
// simd::float4x4 yRotation();
} // namespace Transformation

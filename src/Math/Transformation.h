#pragma once
#include <simd/simd.h>

/**
 * SIMD stands for single instruction multiple data
 * It is required for mathmatics to be fast
 * You want to use 120 bit-wide registers for 4x4 matrices, for example 
 * That way the calculation can be done in a single GPU instruction
 * Instead of multiplying the floats one at the time
 *
 * You need to use compiler intrinsics
 :wa
:qa

:qa
dfdsf
 *
**/
namespace Transformation {

simd::float4x4 identity();
simd::float4x4 translation(simd::float3);
simd::float4x4 zRotation(float theta);
simd::float4x4 xRotation(float theta);
simd::float4x4 yRotation(float theta);
simd::float4x4 rotate(simd::float3 axis);
simd::float4x4 scale(float factor);
simd::float4x4 perspective(float fov, float aspectRatio, float nearZ, float farZ);
// simd::float4x4 xRotation();
// simd::float4x4 yRotation();
} // namespace Transformation

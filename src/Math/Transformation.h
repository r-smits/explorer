#pragma once
#include "Metal/MTLAccelerationStructureTypes.hpp"
#include <simd/simd.h>

/**
 * SIMD stands for single instruction multiple data
 * It is required for mathmatics to be fast
 * You want to use 120 bit-wide registers for 4x4 matrices, for example
 * That way the calculation can be done in a single GPU instruction
 * Instead of multiplying the floats one at the time
 *
 * You need to use compiler intrinsics
 *
 **/

namespace EXP {
namespace MATH {

simd::float4x4 identity();
simd::float4x4 translation(const simd::float3& pos);
simd::float4x4 zRotation(const float& theta);
simd::float4x4 xRotation(const float& theta);
simd::float4x4 yRotation(const float& theta);
simd::float4x4 rotate(const simd::float3& axis);
simd::float4x4 scale(const float& factor);
simd::float4x4 perspective(const float& fov, const float& aspectRatio, const float& nearZ, const float& farZ);
simd::float4x4 orthographic(
    const float& left,
    const float& right,
    const float& bottom,
    const float& top,
    const float& nearZ,
    const float& farZ
);
simd::float4x4 lookat(const simd::float3& eye, const simd::float3& ref, const simd::float3& up);
simd::float4x4 lookat2(const simd::float3& eye, const simd::float3& center, const simd::float3& up);
simd::float4x4 rotate(const float& angle, const simd::float3& axis);
simd::quatf cross(const simd::quatf& a, const simd::quatf& b);
MTL::PackedFloat4x3 pack(const simd::float4x4& m4x4);
} // namespace MATH

} // namespace EXP

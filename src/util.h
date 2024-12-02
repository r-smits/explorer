#pragma once
#include <pch.h>
#include <simd/simd.h>

namespace EXP {
NS::String* nsString(std::string str);
NS::URL* nsUrl(std::string path);
void printError(NS::Error* error);
void print(simd::float2 n);
void print(simd::float3 n);
void print(simd::float4 n);
void print(simd::float4x4 m);
void print(simd::quatf q);


std::string simd2str(const simd::float3& vec3);
std::string simd2str(const simd::float4& vec4);


}; // namespace EXP

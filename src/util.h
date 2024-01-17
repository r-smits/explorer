#pragma once
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {
NS::String* nsString(std::string str);
NS::URL* nsUrl(std::string path);
void printError(NS::Error* error);
void print(simd::float2 n);
void print(simd::float3 n);
void print(simd::float4 n);
void print(simd::float4x4 m);
void print(simd::quatf q);
}; // namespace Explorer

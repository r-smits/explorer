#pragma once
#include <pch.h>
#include <Math/Transformation.h>

namespace EXP {
struct Object {
public:
  Object();
  ~Object(){};

public:
  virtual Object* f4x4();
  virtual Object* translate(const simd::float3& pos);
  virtual Object* scale(const float& factor);
  virtual Object* rotate(const simd::float4x4& rotation);
  virtual const simd::float4x4& get() { return orientation; }

public:
  simd::float4x4 rotation;
  simd::float4x4 orientation;
  simd::float3 position;
  float factor;
};
};


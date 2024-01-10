#pragma once
#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <View/ViewAdapter.hpp>
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {

class Camera : public Object {
public:
  Camera();
  ~Camera() {};

public:
  virtual Camera* f4x4() override;
  virtual simd::float4x4 projection();
  virtual simd::float4x4 model();

private:
  float fov;
  float aspectRatio;
  float nearZ;
  float farZ;

public:
	float rotateSpeed;
};

}; // namespace Explorer

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
  ~Camera(){};

public:
  virtual Camera* f4x4() override;
  virtual Camera* project();

protected:
  simd::float4x4 mProjection;

public:
  float rotateSpeed;
};

class VCamera : public Camera {

	public:
		VCamera();
		~VCamera() {};
	
		const simd::float3 rayDirections() const;

	private:
		simd::float3 position;
		simd::float3 direction;



};

class DefaultCamera : public Camera {

public:
  DefaultCamera();
  ~DefaultCamera(){};

public:
  virtual DefaultCamera* project() override;

public:
  void setFov(float fov);
  void setAspectRatio(float aspectRatio);
  void setNearZ(float nearZ);
  void setFarZ(float farZ);

private:
  float fov;
  float aspectRatio;
  float nearZ;
  float farZ;
};

class OrthographicCamera : public Camera {

public:
  OrthographicCamera();
  ~OrthographicCamera(){};

public:
  virtual OrthographicCamera* project() override;

public:
  void setLeft(float left);
  void setRight(float right);
  void setTop(float top);
  void setBottom(float bottom);

private:
  float left;
  float right;
  float top;
  float bottom;
	float nearZ;
	float farZ;
};

}; // namespace Explorer

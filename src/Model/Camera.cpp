#include "Math/Transformation.h"
#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace Explorer {


	VCamera::VCamera() : position(simd::float3(0.0f)), direction(simd::float3(0.0f)) {}
				


Camera::Camera() : rotateSpeed(1.0f){};

Camera* Camera::project() {
  mProjection = simd::float4x4(1);
  return this;
}

Camera* Camera::f4x4() {
  orientation = mProjection * Transformation::translation(position) * rotation *
                Transformation::scale(factor);
  return this;
}

DefaultCamera::DefaultCamera() : fov(45.0f), nearZ(0.1f), farZ(1000.0f) {
  CGRect bounds = ViewAdapter::bounds();
  this->aspectRatio = bounds.size.width / bounds.size.height;
}

DefaultCamera* DefaultCamera::project() {
  mProjection = Transformation::perspective(fov, aspectRatio, nearZ, farZ);
  return this;
}

void DefaultCamera::setFov(float fov) { this->fov = fov; };
void DefaultCamera::setAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; };
void DefaultCamera::setNearZ(float nearZ) { this->nearZ = nearZ; };
void DefaultCamera::setFarZ(float farZ) { this->farZ = farZ; };

OrthographicCamera::OrthographicCamera()
    : left(-1.0f), right(1.0f), bottom(-1.0f), top(1.0f), nearZ(10.0f), farZ(-10.0f) {
CGRect frame = ViewAdapter::bounds();
		this->left = -frame.size.width / 1000;
		this->right = frame.size.width / 1000;
		this->top = frame.size.height / 1000;
		this->bottom = -frame.size.height / 1000;

		}

OrthographicCamera* OrthographicCamera::project() {
	mProjection = Transformation::orthographic(left, right, bottom, top, nearZ, farZ);
	return this;
}

void OrthographicCamera::setLeft(float left) { this->left = left; }
void OrthographicCamera::setRight(float right) { this->right = right; }
void OrthographicCamera::setTop(float top) { this->top = top; }
void OrthographicCamera::setBottom(float bottom) { this->bottom = bottom; }
} // namespace Explorer

//= Transformation::perspective(45.0f, 1.0f, 0.1f, 100.0f);

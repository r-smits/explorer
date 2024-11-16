#include "Math/Transformation.h"
#include <Events/IOState.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace Explorer {

VCamera::VCamera() {


	speed = {.05, .05 * .25};
  vUp = {0.0f, 1.0f, 0.0f};
  vForward = {0.0f, 0.0f, -1.0f};
	rotationY = simd::quatf(speed.y, vUp);
	nRotationY = simd::quatf(-speed.y, vUp);

  CGRect frame = ViewAdapter::bounds();
  resolution = {(float)frame.size.width * 2, (float)frame.size.height * 2};
  float fov = frame.size.width / frame.size.height;
  lastMousePos = IO::getMouse() / resolution * 2 - 1;

  simd::float3 vOrigin = {0.0f, 0.0f, 2.0f};

	//simd::float4x4 mProjection = Transformation::orthographic(-1, 1, -1, 1, 1, -1);
	simd::float4x4 mProjection = Transformation::perspective(45.0f, fov, 0.1f, 100.0f);
  
	simd::float4x4 mInverseProjection = simd::inverse(mProjection);
  simd::float4x4 mView = Transformation::lookat(vOrigin, vOrigin + vForward, vUp);
  simd::float4x4 mInverseView = simd::inverse(mView);
  
	rTransform = {mProjection, mView, mInverseProjection, mInverseView, vOrigin};
};

simd::float3 VCamera::vRight() { return simd::cross(vForward, vUp); }

const void VCamera::setSpeed(float speed) { this->speed = speed; }

void VCamera::Iso() {
	simd::quatf rotX = simd::quatf(-45.0f * M_PI/180, vRight());
	simd::quatf rotY = simd::quatf(-45.0f * M_PI/180, vUp);
	simd::quatf angle = simd::normalize(rotX * rotY);
	vForward = simd_act(angle, vForward);
	rTransform.rayOrigin = simd_act(angle, rTransform.rayOrigin);
}

Renderer::RTTransform VCamera::update() {

  if (IO::isPressed(KEY_W)) rTransform.rayOrigin += vForward * speed.x;
  if (IO::isPressed(KEY_S)) rTransform.rayOrigin -= vForward * speed.x;
  if (IO::isPressed(KEY_A)) rTransform.rayOrigin -= vRight() * speed.x;
  if (IO::isPressed(KEY_D)) rTransform.rayOrigin += vRight() * speed.x;
  if (IO::isPressed(KEY_Q)) rTransform.rayOrigin -= vUp * speed.x;
  if (IO::isPressed(KEY_E)) rTransform.rayOrigin += vUp * speed.x;
	
	if (IO::isPressed(KEY_J)) {
		vForward = simd_act(rotationY, vForward);
		rTransform.rayOrigin = simd_act(rotationY, rTransform.rayOrigin);
	}

	if (IO::isPressed(KEY_K)) {
		vForward = simd_act(nRotationY, vForward);
		rTransform.rayOrigin = simd_act(nRotationY, rTransform.rayOrigin);
	}

  if (IO::isPressed(ARROW_LEFT)) vForward = simd_act(rotationY, vForward);
  if (IO::isPressed(ARROW_RIGHT)) vForward = simd_act(nRotationY, vForward);

  simd::float3 center = rTransform.rayOrigin + vForward;
  rTransform.mView = Transformation::lookat(rTransform.rayOrigin, center, vUp);
  rTransform.mInverseView = simd::inverse(rTransform.mView);
  return rTransform;
}

void VCamera::updateView() {}

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


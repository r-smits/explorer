#include <Events/IOState.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace EXP {

VCamera::VCamera() {
	
	CGRect frame = ViewAdapter::bounds();
	resolution = {(float)frame.size.width * 2, (float)frame.size.height * 2};
	speed = {.05, .05 * .25};
  lastMousePos = IO::getMouse() / resolution.xy * 2 - 1;

  vecOrigin = {.0f, .0f, 2.0f};
	vecUp = {.0f, 1.0f, .0f};
  vecRight = {1.0f, .0f, .0f};
  vecForward = {.0f, .0f, -1.0f};
	
	quatPosY = simd::quatf(speed.y, vecUp);
	quatNegY = simd::quatf(-speed.y, vecUp);

	//simd::float4x4 mProjection = EXP::MATH::orthographic(-1, 1, -1, 1, 1, -1);
	projection = EXP::MATH::perspective(45.0f, frame.size.width / frame.size.height, 0.1f, 100.0f);
	setIsometric();
};

const simd::float3& VCamera::getVRight() { 
	vecRight = simd::cross(vecForward, vecUp);
	return vecRight; 
}

const void VCamera::setSpeed(const float& speed) { 
	this->speed = speed; 
}

const void VCamera::setIsometric() {
	simd::quatf rotX = simd::quatf(-45.0f * M_PI/180, this->getVRight());
	simd::quatf rotY = simd::quatf(-45.0f * M_PI/180, vecUp);
	simd::quatf angle = simd::normalize(rotX * rotY);
	
	vecForward = simd_act(angle, vecForward);
	vecOrigin = simd_act(angle, vecOrigin);
  flush_to_gpu();
}

const Renderer::VCamera& VCamera::update() {
  if (IO::isPressed(KEY_W)) vecOrigin += vecForward * speed.x;
  if (IO::isPressed(KEY_S)) vecOrigin -= vecForward * speed.x;
  if (IO::isPressed(KEY_A)) vecOrigin -= getVRight() * speed.x;
  if (IO::isPressed(KEY_D)) vecOrigin += getVRight() * speed.x;
  if (IO::isPressed(KEY_Q)) vecOrigin -= vecUp * speed.x;
  if (IO::isPressed(KEY_E)) vecOrigin += vecUp * speed.x;

  if (IO::isPressed(KEY_J)) {
      vecForward = simd_act(quatPosY, vecForward);
      vecOrigin  = simd_act(quatPosY, vecOrigin);
  }
  if (IO::isPressed(KEY_K)) {
      vecForward = simd_act(quatNegY, vecForward);
      vecOrigin  = simd_act(quatNegY, vecOrigin);
  }
  if (IO::isPressed(ARROW_LEFT))  vecForward = simd_act(quatPosY, vecForward);
  if (IO::isPressed(ARROW_RIGHT)) vecForward = simd_act(quatNegY, vecForward);

  // Recompute right from current forward/up before flushing
  vecRight = simd::cross(vecForward, vecUp);
  flush_to_gpu();
  return vcamera;
}

const void VCamera::flush_to_gpu() {
  this->vcamera = {
    .orientation = EXP::MATH::lookat(vecOrigin, vecOrigin + vecForward, vecUp), // ← not stale `view`
    .vecOrigin   = {vecOrigin.x,  vecOrigin.y,  vecOrigin.z,  1.0f},
    .resolution  = {resolution.x, resolution.y, 0.0f,         0.0f},
    .vecRight    = {vecRight.x,   vecRight.y,   vecRight.z,   0.0f},
    .vecUp       = {vecUp.x,      vecUp.y,      vecUp.z,      0.0f},
    .vecForward  = {vecForward.x, vecForward.y, vecForward.z, 0.0f},
    .fovScale    = tanf(45.0f * M_PI / 180.0f * 0.5f)
  };
}

const void VCamera::updateView() {}


Camera::Camera() : rotateSpeed(1.0f) {};

Camera* Camera::project() {
  mProjection = simd::float4x4(1);
  return this;
}

Camera* Camera::f4x4() {
  this->orientation = mProjection * EXP::MATH::translation(position) * rotation * EXP::MATH::scale(factor);
  return this;
}

DefaultCamera::DefaultCamera() : fov(45.0f), nearZ(0.1f), farZ(1000.0f) {
  CGRect bounds = ViewAdapter::bounds();
  this->aspectRatio = bounds.size.width / bounds.size.height;
}

DefaultCamera* DefaultCamera::project() {
  this->mProjection = EXP::MATH::perspective(fov, aspectRatio, nearZ, farZ);
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
  this->mProjection = EXP::MATH::orthographic(left, right, bottom, top, nearZ, farZ);
  return this;
}

void OrthographicCamera::setLeft(float left) { this->left = left; }
void OrthographicCamera::setRight(float right) { this->right = right; }
void OrthographicCamera::setTop(float top) { this->top = top; }
void OrthographicCamera::setBottom(float bottom) { this->bottom = bottom; }
} // namespace EXP


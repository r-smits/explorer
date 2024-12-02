#include <Events/IOState.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace EXP {

VCamera::VCamera() {
	
	CGRect frame = ViewAdapter::bounds();
	simd::float3 resolution = {(float)frame.size.width * 2, (float)frame.size.height * 2, 1.0f};

	this->speed = {.05, .05 * .25};
  this->lastMousePos = IO::getMouse() / resolution.xy * 2 - 1;

	simd::float3 vecOrigin = {0.0f, 0.0f, 2.0f};
	this->vecUp = {0.0f, 1.0f, 0.0f};
  this->vecForward = {0.0f, 0.0f, -1.0f};
	
	this->quatPosY = simd::quatf(this->speed.y, this->vecUp);
	this->quatNegY = simd::quatf(-this->speed.y, this->vecUp);

	//simd::float4x4 mProjection = EXP::MATH::orthographic(-1, 1, -1, 1, 1, -1);
	this->projection = simd::inverse(EXP::MATH::perspective(45.0f, frame.size.width / frame.size.height, 0.1f, 100.0f));
	this->view = EXP::MATH::lookat(vecOrigin, vecOrigin + this->vecForward, this->vecUp);

	this->transforms = {this->view * this->projection, vecOrigin, resolution};
};

const simd::float3& VCamera::getVRight() { 
	this->vecRight = simd::cross(this->vecForward, this->vecUp);
	return this->vecRight; 
}

const void VCamera::setSpeed(const float& speed) { 
	this->speed = speed; 
}

const void VCamera::setIsometric() {
	simd::quatf rotX = simd::quatf(-45.0f * M_PI/180, this->getVRight());
	simd::quatf rotY = simd::quatf(-45.0f * M_PI/180, this->vecUp);
	simd::quatf angle = simd::normalize(rotX * rotY);
	
	this->vecForward = simd_act(angle, this->vecForward);
	this->transforms.vecOrigin = simd_act(angle, this->transforms.vecOrigin);
}

const Renderer::VCamera& VCamera::update() {

  if (IO::isPressed(KEY_W)) this->transforms.vecOrigin += this->vecForward * this->speed.x;
  if (IO::isPressed(KEY_S)) this->transforms.vecOrigin -= this->vecForward * this->speed.x;
  if (IO::isPressed(KEY_A)) this->transforms.vecOrigin -= this->getVRight() * this->speed.x;
  if (IO::isPressed(KEY_D)) this->transforms.vecOrigin += this->getVRight() * this->speed.x;
  if (IO::isPressed(KEY_Q)) this->transforms.vecOrigin -= this->vecUp * this->speed.x;
  if (IO::isPressed(KEY_E)) this->transforms.vecOrigin += this->vecUp * this->speed.x;
	
	if (IO::isPressed(KEY_J)) {
		this->vecForward = simd_act(this->quatPosY, this->vecForward);
		this->transforms.vecOrigin = simd_act(this->quatPosY, this->transforms.vecOrigin);
	}

	if (IO::isPressed(KEY_K)) {
		this->vecForward = simd_act(this->quatNegY, this->vecForward);
		this->transforms.vecOrigin = simd_act(this->quatNegY, this->transforms.vecOrigin);
	}

  if (IO::isPressed(ARROW_LEFT)) this->vecForward = simd_act(this->quatPosY, this->vecForward);
  if (IO::isPressed(ARROW_RIGHT)) this->vecForward = simd_act(this->quatNegY, this->vecForward);
	
  simd::float3 center = this->transforms.vecOrigin + this->vecForward;
  this->view = EXP::MATH::lookat(this->transforms.vecOrigin, center, this->vecUp);

	this->transforms.orientation = this->view * this->projection;
  return transforms;
}

const void VCamera::updateView() {}

/**
const simd::float4x4& VCamera::getOrientation() {
	this->update();	
	this->orientation = this->rTransform.mView * this->rTransform.mProjection;
	//this->orientation = this->rTransform.mInverseProjection * this->rTransform.mView;
	return this->orientation;
}
**/

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


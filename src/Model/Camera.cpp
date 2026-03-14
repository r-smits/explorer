#include <Events/IOState.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace EXP {

  const float RAD_45 = 45.0f * M_PI / 180.0f;


  Camera::Camera() : rotateSpeed(1.0f) {};

Camera* Camera::project() {
  mProjection = simd::float4x4(1);
  return this;
}

Camera* Camera::f4x4() {
  orientation = mProjection * EXP::MATH::translation(position) * rotation * EXP::MATH::scale(factor);
  return this;
}

VCamera::VCamera() {
	
	CGRect frame = ViewAdapter::bounds();

  resolution = {
    static_cast<float>(frame.size.width) * 2, 
    static_cast<float>(frame.size.height) * 2, 
    1.0f
  };
  
	speed = {.05, .05 * .25};
  lastMousePos = IO::getMouse() / resolution.xy * 2 - 1;

	vecOrigin = {0.0f, 0.0f, 3.0f};
	vecUp = {0.0f, 1.0f, 0.0f};
  vecForward = {0.0f, 0.0f, -1.0f};
  vecRight = getVRight();
  fovScale = tanf(RAD_45 * 0.5f);
	
	quatPosY = simd::quatf(speed.y, vecUp);
	quatNegY = simd::quatf(-speed.y, vecUp);

	//simd::float4x4 mProjection = EXP::MATH::orthographic(-1, 1, -1, 1, 1, -1);
	projection = simd::inverse(EXP::MATH::perspective(45.0f, frame.size.width / frame.size.height, 0.1f, 100.0f));
	view = EXP::MATH::lookat(vecOrigin, vecOrigin + vecForward, vecUp);
	transforms = {
    .vecOrigin = MATH::pack(vecOrigin),
    .resolution = MATH::pack(resolution),
    .vecRight = MATH::pack(vecRight),
    .vecUp = MATH::pack(vecUp),
    .vecForward = MATH::pack(vecForward),
    .fovScale = fovScale
  };
};

const simd::float3& VCamera::getVRight() { 
	vecRight = simd::cross(vecForward, vecUp);
	return vecRight; 
}

const void VCamera::setSpeed(const float& _speed) { 
	speed =_speed; 
}

const void VCamera::setIsometric() {
	simd::quatf rotX = simd::quatf(-RAD_45, getVRight());
	simd::quatf rotY = simd::quatf(-RAD_45, vecUp);
	simd::quatf angle = simd::normalize(rotX * rotY);
	vecForward = simd_act(angle, vecForward);
  vecOrigin = simd_act(angle, vecOrigin);
  vecRight = getVRight();
	transforms = {
    .vecOrigin = MATH::pack(vecOrigin),
    .resolution = MATH::pack(resolution),
    .vecRight = MATH::pack(vecRight),
    .vecUp = MATH::pack(vecUp),
    .vecForward = MATH::pack(vecForward),
    .fovScale = fovScale
  };
}

const Renderer::VCamera& VCamera::update() {
  if (IO::isPressed(KEY_W)) fovScale -= speed.x;
  if (IO::isPressed(KEY_S)) fovScale += speed.x;
  // if (IO::isPressed(KEY_W)) vecOrigin += vecForward * speed.x;
  // if (IO::isPressed(KEY_S)) vecOrigin -= vecForward * speed.x;
  if (IO::isPressed(KEY_A)) vecOrigin -= getVRight() * speed.x;
  if (IO::isPressed(KEY_D)) vecOrigin += getVRight() * speed.x;
  if (IO::isPressed(KEY_Q)) vecOrigin -= vecUp * speed.x;
  if (IO::isPressed(KEY_E)) vecOrigin += vecUp * speed.x;
	
	if (IO::isPressed(KEY_J)) {
		vecForward = simd_act(quatPosY, vecForward);
		vecOrigin = simd_act(quatPosY, vecOrigin);
	}

	if (IO::isPressed(KEY_K)) {
		vecForward = simd_act(quatNegY, vecForward);
		vecOrigin = simd_act(quatNegY, vecOrigin);
	}

  if (IO::isPressed(ARROW_LEFT) || IO::isPressed(ARROW_RIGHT)) {
    simd::quatf rot = IO::isPressed(ARROW_LEFT) ? quatPosY : quatNegY;
    vecForward = simd::normalize(simd_act(rot, vecForward));
  }

  vecRight = getVRight();
	transforms = {
    .vecOrigin = MATH::pack(vecOrigin),
    .resolution = MATH::pack(resolution),
    .vecRight = MATH::pack(vecRight),
    .vecUp = MATH::pack(vecUp),
    .vecForward = MATH::pack(vecForward),
    .fovScale = fovScale
  };
  return transforms;
}

const Renderer::VCamera& VCamera::get() { return transforms; }
const void VCamera::updateView() {}

};

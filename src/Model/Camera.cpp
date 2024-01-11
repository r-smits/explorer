#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace Explorer {

Camera::Camera() : fov(45.0f), nearZ(0.1f), farZ(1000.0f), rotateSpeed(1.0f) {
  CGRect bounds = ViewAdapter::bounds();
  this->aspectRatio = bounds.size.width / bounds.size.height;
}

Explorer::Camera* Camera::f4x4() {
  orientation = projection() * Transformation::translation(position) * rotation *
                Transformation::scale(factor);
	return this;
}

simd::float4x4 Camera::projection() {
  return Transformation::perspective(fov, aspectRatio, nearZ, farZ);
}

simd::float4x4 Camera::model() { return simd::float4x4(1); }

} // namespace Explorer

//= Transformation::perspective(45.0f, 1.0f, 0.1f, 100.0f);

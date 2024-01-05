#include <Model/Camera.h>
#include <Model/MeshFactory.h>

namespace Explorer {

Camera::Camera() {
  this->fov = 45;
  CGRect bounds = ViewAdapter::bounds();
  this->aspectRatio = bounds.size.width / bounds.size.height;
  this->nearZ = 0.1;
  this->farZ = 100;
}

simd::float4x4 Camera::f4x4() {
  return projection() * Transformation::translation(position) * rotation *
         Transformation::scale(scale);
}

simd::float4x4 Camera::projection() {
  return Transformation::perspective(fov, aspectRatio, nearZ, farZ);
}

simd::float4x4 Camera::model() { return simd::float4x4(1); }

} // namespace Explorer

//= Transformation::perspective(45.0f, 1.0f, 0.1f, 100.0f);

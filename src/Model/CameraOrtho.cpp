// #include <Events/IOState.h>
// #include <Model/CameraOrtho.h>
// #include <Model/MeshFactory.h>




// DefaultCamera::DefaultCamera() : fov(45.0f), nearZ(0.1f), farZ(1000.0f) {
//   CGRect bounds = ViewAdapter::bounds();
//   aspectRatio = bounds.size.width / bounds.size.height;
// }

// DefaultCamera* DefaultCamera::project() {
//   mProjection = EXP::MATH::perspective(fov, aspectRatio, nearZ, farZ);
//   return this;
// }

// void DefaultCamera::setFov(float fov) { fov = fov; };
// void DefaultCamera::setAspectRatio(float aspectRatio) { aspectRatio = aspectRatio; };
// void DefaultCamera::setNearZ(float nearZ) { nearZ = nearZ; };
// void DefaultCamera::setFarZ(float farZ) { farZ = farZ; };

// OrthographicCamera::OrthographicCamera()
//     : left(-1.0f), right(1.0f), bottom(-1.0f), top(1.0f), nearZ(10.0f), farZ(-10.0f) {
//   CGRect frame = ViewAdapter::bounds();
//   left = -frame.size.width / 1000;
//   right = frame.size.width / 1000;
//   top = frame.size.height / 1000;
//   bottom = -frame.size.height / 1000;
// }

// OrthographicCamera* OrthographicCamera::project() {
//   mProjection = EXP::MATH::orthographic(left, right, bottom, top, nearZ, farZ);
//   return this;
// }

// void OrthographicCamera::setLeft(float _left) { left = _left; }
// void OrthographicCamera::setRight(float _right) { right = _right; }
// void OrthographicCamera::setTop(float _top) { top = _top; }
// void OrthographicCamera::setBottom(float _bottom) { bottom = _bottom; }
// } // namespace EXP


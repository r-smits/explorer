#pragma once
#include <Model/Camera.h>
#include <Model/MeshFactory.h>
#include <pch.h>

namespace Renderer {

class Draw {
public:
  Draw();
  ~Draw();

  static void
  light(MTL::RenderCommandEncoder* encoder, EXP::Camera camera, EXP::Light* light);
  static void
  model(MTL::RenderCommandEncoder* encoder, EXP::Camera camera, EXP::Model* model);
};

}; // namespace Renderer

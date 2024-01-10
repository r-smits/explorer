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
  light(MTL::RenderCommandEncoder* encoder, Explorer::Camera camera, Explorer::Light* light);
  static void
  model(MTL::RenderCommandEncoder* encoder, Explorer::Camera camera, Explorer::Model* model);
};

}; // namespace Renderer

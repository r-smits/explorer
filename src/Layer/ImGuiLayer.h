#pragma once
#include <Layer/Layer.h>
#include <pch.h>

namespace Explorer {

class ImGuiLayer : public Layer {

public:
  ImGuiLayer(MTK::View *view);
  ~ImGuiLayer();

  void onAttach(MTK::View *view);
  void onDetach() override;
  void onUpdate(MTK::View *view, MTL::RenderCommandEncoder *encoder) override;
  void onEvent(Event &event) override;
};

}; // namespace Explorer

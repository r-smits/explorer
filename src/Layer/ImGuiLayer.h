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

public: // Event
  void onEvent(Event &event) override;
  virtual bool onKeyPressed(KeyPressedEvent &event);
  virtual bool onKeyReleased(KeyReleasedEvent &event);
  virtual bool onMouseButtonPressed(MouseButtonPressedEvent &event);
  virtual bool onMouseButtonReleased(MouseButtonReleasedEvent &event);
  virtual bool onMouseMove(MouseMoveEvent &event);
};

}; // namespace Explorer

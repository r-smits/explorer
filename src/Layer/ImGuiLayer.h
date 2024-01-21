#pragma once
#include <Layer/Layer.h>
#include <pch.h>

namespace Explorer {

class ImGuiLayer : public Layer {

public:
  ImGuiLayer(MTK::View *view, AppProperties* config);
  ~ImGuiLayer();
  void onAttach(MTK::View *view);
  void onDetach() override;
  void onUpdate(MTK::View *view, MTL::CommandBuffer* buffer) override;

public:
	void showDebugWindow(const bool& open);

public: // Event
  void onEvent(Event &event) override;
  virtual bool onKeyPressed(KeyPressedEvent &event);
  virtual bool onKeyReleased(KeyReleasedEvent &event);
  virtual bool onMouseButtonPressed(MouseButtonPressedEvent &event);
  virtual bool onMouseButtonReleased(MouseButtonReleasedEvent &event);
  virtual bool onMouseMove(MouseMoveEvent &event);

private:
	MTL::CommandQueue* queue;
};

}; // namespace Explorer

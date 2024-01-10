#pragma once
#include <Layer/LayerStack.h>
#include <pch.h>
#include <Control/AppProperties.h>

// ViewDelegate with extensions from objc

namespace Explorer {

class ViewDelegate : public MTK::ViewDelegate {

public: // ViewDelegate overrides
  ViewDelegate(MTK::View* view, AppProperties* config);
  virtual ~ViewDelegate() override;
  virtual void drawInMTKView(MTK::View* view) override;
  virtual void drawableSizeWillChange(MTK::View* view, CGSize) override;

public: // Event
  virtual void onEvent(Event& event);
  virtual bool onKeyPressed(KeyPressedEvent& event);
  virtual bool onKeyReleased(KeyReleasedEvent& event);
  virtual bool onMouseButtonPressed(MouseButtonPressedEvent& event);
  virtual bool onMouseButtonReleased(MouseButtonReleasedEvent& event);
  virtual bool onMouseMove(MouseMoveEvent& event);

private:
  MTL::CommandQueue* queue;
  LayerStack layerStack;
};
} // namespace Explorer

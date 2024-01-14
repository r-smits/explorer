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

private:
  MTL::CommandQueue* queue;
  LayerStack layerStack;
};
} // namespace Explorer

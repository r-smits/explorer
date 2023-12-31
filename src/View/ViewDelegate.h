#pragma once
#include "Metal/MTLCommandQueue.hpp"
#include <Events/Events.h>
#include <Events/KeyEvent.h>
#include <Layer/LayerStack.h>
#include <Renderer/Renderer.h>
#include <pch.h>

// ViewDelegate with extensions from objc

#define BIND_EVENT(fn) std::bind(&Explorer::ViewDelegate::fn, this, std::placeholders::_1)

namespace Explorer {

class ViewDelegate : public MTK::ViewDelegate {

public:
  ViewDelegate(MTK::View *view);
  virtual ~ViewDelegate() override;
  virtual void drawInMTKView(MTK::View *view) override;
  virtual void drawableSizeWillChange(MTK::View *view, CGSize) override;
  virtual void onEvent(Event &event);
  virtual bool onKeyPressed(KeyPressedEvent &event);
  virtual bool onKeyReleased(KeyReleasedEvent &event);
  virtual void pushLayer(Layer *layer);
  virtual void pushOverlay(Layer *layer);

private:
	MTL::CommandQueue* queue;
  LayerStack layerStack;
  Renderer *renderer;
};
} // namespace Explorer

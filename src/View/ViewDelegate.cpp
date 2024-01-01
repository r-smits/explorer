#include <Layer/BaseLayer.h>
#include <Layer/ImGuiLayer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>

Explorer::ViewDelegate::ViewDelegate(MTK::View *view) : MTK::ViewDelegate() {

  this->queue = view->device()->newCommandQueue();

  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter *viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event &event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);
  DEBUG("Initializing ViewDelegate ...");

  // Initialize layers & renderer (will be a layer in the future)
  this->layerStack.pushLayer(new BaseLayer(view->device()));
  this->layerStack.pushOverlay(new ImGuiLayer(view));
}

Explorer::ViewDelegate::~ViewDelegate() {}

void Explorer::ViewDelegate::onEvent(Event &event) {
  
  for (std::vector<Layer *>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled())
      break;
  }
}

bool Explorer::ViewDelegate::onMouseButtonPressed(MouseButtonPressedEvent &event) {
  //DEBUG(event.toString());
  return true;
}

bool Explorer::ViewDelegate::onMouseButtonReleased(MouseButtonReleasedEvent &event) {
  //DEBUG(event.toString());
  return true;
}

bool Explorer::ViewDelegate::onMouseMove(MouseMoveEvent &event) {
  //DEBUG(event.toString());
  return true;
}
bool Explorer::ViewDelegate::onKeyPressed(KeyPressedEvent &event) {
  //DEBUG(event.toString());
  return true;
}

bool Explorer::ViewDelegate::onKeyReleased(KeyReleasedEvent &event) {
  //DEBUG(event.toString());
  return true;
}

void Explorer::ViewDelegate::drawInMTKView(MTK::View *view) {
  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
  MTL::CommandBuffer *buffer = queue->commandBuffer();
  MTL::RenderPassDescriptor *descriptor = view->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder *encoder = buffer->renderCommandEncoder(descriptor);

  for (Layer *layer : this->layerStack)
    layer->onUpdate(view, encoder);

  encoder->endEncoding();
  buffer->presentDrawable(view->currentDrawable());
  buffer->commit();
  buffer->waitUntilScheduled();
  pool->release();
}

void Explorer::ViewDelegate::drawableSizeWillChange(MTK::View *view, CGSize size) {}

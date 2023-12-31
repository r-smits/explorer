#include "Metal/MTLCommandEncoder.hpp"
#include "Metal/MTLCommandQueue.hpp"
#include <Layer/ImGuiLayer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>

Explorer::ViewDelegate::ViewDelegate(MTK::View *view)
    : MTK::ViewDelegate(), queue(view->device()->newCommandQueue()) {
  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter *viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event &event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);

  // Initialize layers & renderer (will be a layer in the future)
	ImGuiLayer *imguiLayer = new ImGuiLayer(view);
	this->renderer = new Renderer(view->device(), imguiLayer);
  this->pushOverlay(imguiLayer);
}

Explorer::ViewDelegate::~ViewDelegate() {
	delete this->renderer;
}

void Explorer::ViewDelegate::onEvent(Explorer::Event &event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(onKeyReleased));

  for (std::vector<Layer *>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled())
      break;
  }
}

bool Explorer::ViewDelegate::onKeyPressed(KeyPressedEvent &event) {
  DEBUG(event.toString());
  return true;
}

bool Explorer::ViewDelegate::onKeyReleased(KeyReleasedEvent &event) {
  DEBUG(event.toString());
  return true;
}

void Explorer::ViewDelegate::pushLayer(Layer *layer) { this->layerStack.pushLayer(layer); }

void Explorer::ViewDelegate::pushOverlay(Layer *layer) { this->layerStack.pushOverlay(layer); }

void Explorer::ViewDelegate::drawInMTKView(MTK::View *view) {
	NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();
	MTL::CommandBuffer* buffer = queue->commandBuffer();
	MTL::RenderPassDescriptor *descriptor = view->currentRenderPassDescriptor();
	MTL::RenderCommandEncoder* encoder = buffer->renderCommandEncoder(descriptor);

	renderer->draw(view, encoder);
  for (Layer *layer : this->layerStack)
    layer->onUpdate(view, encoder);

	encoder->endEncoding();
  buffer->presentDrawable(view->currentDrawable());
  buffer->commit();
	pool->release();
}

void Explorer::ViewDelegate::drawableSizeWillChange(MTK::View *view, CGSize size) {}

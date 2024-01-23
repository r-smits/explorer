#include <Layer/RayTraceLayer.h>
#include <Events/IOState.h>
#include <Layer/BaseLayer.h>
#include <Layer/ImGuiLayer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>

Explorer::ViewDelegate::ViewDelegate(MTK::View* view, AppProperties* config) : MTK::ViewDelegate() {
  DEBUG("Initializing ViewDelegate ...");
  this->queue = view->device()->newCommandQueue();

  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter* viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event& event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);

  // Initialize layers & renderer (will be a layer in the future)
  this->layerStack.pushLayer(new BaseLayer(view->device(), config));
  // this->layerStack.pushOverlay(new ImGuiLayer(view, config));

	//this->layerStack.pushLayer(new RayTraceLayer(view->device(), config));
}

Explorer::ViewDelegate::~ViewDelegate() {}

void Explorer::ViewDelegate::onEvent(Event& event) {
  IO::onEvent(event);
  for (std::vector<Layer*>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled()) break;
  }
}

void Explorer::ViewDelegate::drawInMTKView(MTK::View* view) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
	

  MTL::CommandBuffer* buffer = queue->commandBuffer();
  MTL::RenderPassDescriptor* descriptor = view->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* encoder = buffer->renderCommandEncoder(descriptor);

  for (Layer* layer : this->layerStack)
    layer->onUpdate(view, encoder);

  encoder->endEncoding();
  buffer->presentDrawable(view->currentDrawable());
  buffer->commit();
  buffer->waitUntilScheduled();
  pool->release();
}

void Explorer::ViewDelegate::drawableSizeWillChange(MTK::View* view, CGSize size) {}

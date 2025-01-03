#include <Layer/RayTraceLayer.h>
#include <Events/IOState.h>
#include <Layer/BaseLayer.h>
#include <Layer/ImGuiLayer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>

EXP::ViewDelegate::ViewDelegate(MTK::View* view, EXP::AppProperties* config) : MTK::ViewDelegate() {
  DEBUG("Initializing ViewDelegate ...");

  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter* viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event& event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);
	this->layerStack.pushLayer(new EXP::RayTraceLayer(view->device(), config));
}

EXP::ViewDelegate::~ViewDelegate() {}

void EXP::ViewDelegate::onEvent(Event& event) {
  IO::onEvent(event);
  for (std::vector<Layer*>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled()) break;
  }
}

void EXP::ViewDelegate::drawInMTKView(MTK::View* view) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
	

  //MTL::CommandBuffer* buffer = queue->commandBuffer();
  //MTL::RenderPassDescriptor* descriptor = view->currentRenderPassDescriptor();
  //MTL::RenderCommandEncoder* encoder = buffer->renderCommandEncoder(descriptor);

  for (Layer* layer : this->layerStack)
    layer->onUpdate(view, nullptr);

  //encoder->endEncoding();
  //buffer->presentDrawable(view->currentDrawable());
  //buffer->commit();
  //buffer->waitUntilScheduled();
  //pool->release();
}

void EXP::ViewDelegate::drawableSizeWillChange(MTK::View* view, CGSize size) {}

#include <Layer/RayTraceLayer.h>
#include <Events/IOState.h>
#include <Layer/BaseLayer.h>
#include <Layer/ImGuiLayer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>

EXP::ViewDelegate::ViewDelegate(
	MTK::View* view, 
	std::shared_ptr<const EXP::AppProperties> app_properties
) : MTK::ViewDelegate() {
  DEBUG("Initializing ViewDelegate ...");
  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter* viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event& event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);
	this->layerStack.pushLayer(new EXP::RayTraceLayer(view->device(), app_properties));
	DEBUG("Initialized ViewDelegate");
}

EXP::ViewDelegate::~ViewDelegate() {}

void EXP::ViewDelegate::onEvent(Event& event) {
	DEBUG("ViewDelegate onEvent");
  IO::onEvent(event);
  for (std::vector<Layer*>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled()) break;
  }
}

void EXP::ViewDelegate::drawInMTKView(MTK::View* view) {
	DEBUG("Draw in MTK View");
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
  for (Layer* layer : this->layerStack)
    layer->onUpdate(view, nullptr);
	pool->release();
}


void EXP::ViewDelegate::drawableSizeWillChange(MTK::View* view, CGSize size) {}



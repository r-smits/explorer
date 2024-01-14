#include <Layer/Layer.h>

Explorer::Layer::Layer(MTL::Device *device, AppProperties* config, const std::string &name)
    : device(device), config(config), name(name) {
}
Explorer::Layer::~Layer() {
	device->release();
	delete config;
}
void Explorer::Layer::onEvent(Event &event) {}
void Explorer::Layer::onUpdate(MTK::View *view, MTL::RenderCommandEncoder* buffer) {}
void Explorer::Layer::onAttach() {}
void Explorer::Layer::onDetach() {}

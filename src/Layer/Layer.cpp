#include <Layer/Layer.h>

EXP::Layer::Layer(MTL::Device *device, EXP::AppProperties* config, const std::string &name)
    : device(device), config(config), name(name) {
}
EXP::Layer::~Layer() {
	device->release();
	delete config;
}
void EXP::Layer::onEvent(Event &event) {}
void EXP::Layer::onUpdate(MTK::View *view, MTL::RenderCommandEncoder* buffer) {}
void EXP::Layer::onAttach() {}
void EXP::Layer::onDetach() {}


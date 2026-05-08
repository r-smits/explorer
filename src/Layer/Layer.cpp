#include <Layer/Layer.h>

EXP::Layer::Layer(MTK::View* view, std::shared_ptr<const EXP::AppProperties> config, const std::string &name)
    : device(view->device()), config(config), name(name) {}
	
EXP::Layer::~Layer() {
	device->release();
}
void EXP::Layer::onEvent(Event &event) {}
void EXP::Layer::onUpdate(MTK::View *view, MTL::RenderCommandEncoder* buffer) {}
void EXP::Layer::onAttach() {}
void EXP::Layer::onDetach() {}


#include <Layer/Layer.h>

Explorer::Layer::Layer(MTL::Device *device, const std::string &name)
    : device(device->retain()), queue(device->newCommandQueue()), name(name) {
}
Explorer::Layer::~Layer() {}
void Explorer::Layer::onEvent(Event &event) {}
void Explorer::Layer::onUpdate(MTK::View *view, MTL::RenderCommandEncoder* encoder) {}
void Explorer::Layer::onAttach() {}
void Explorer::Layer::onDetach() {}

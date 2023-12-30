#include <Layer/Layer.h>

Explorer::Layer::Layer(const std::string &name) : name(name) {}

Explorer::Layer::~Layer() {}

void Explorer::Layer::onEvent(Event &event) {}

void Explorer::Layer::onUpdate() {}
void Explorer::Layer::onAttach() {}
void Explorer::Layer::onDetach() {}

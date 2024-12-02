#include <Layer/BaseLayer.h>
#include <Layer/ImGuiLayer.h>
#include <Layer/LayerStack.h>
#include <View/ViewAdapter.hpp>

EXP::LayerStack::LayerStack() {
  DEBUG("Initializing LayerStack ...");
  this->insert = layers.begin();
}

EXP::LayerStack::~LayerStack() {
  for (Layer* layer : layers)
    delete layer;
}

void EXP::LayerStack::pushLayer(Layer* layer) { this->layers.emplace(this->insert, layer); }

void EXP::LayerStack::pushOverlay(Layer* overlay) { this->layers.emplace_back(overlay); }

void EXP::LayerStack::popLayer(Layer* layer) {
  std::vector<Layer*>::iterator position = std::find(layers.begin(), layers.end(), layer);
  if (position != layers.end()) {
    layers.erase(position);
    insert--;
  }
}

void EXP::LayerStack::popOverlay(Layer* overlay) {

  std::vector<Layer*>::iterator position = std::find(layers.begin(), layers.end(), overlay);
  if (position != layers.end()) {
    layers.erase(position);
  }
}

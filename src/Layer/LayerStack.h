#pragma once
#include <Layer/Layer.h>
#include <pch.h>

namespace Explorer {

class LayerStack {

public:
  LayerStack();
  ~LayerStack();

  void pushLayer(Layer *layer);
  void pushOverlay(Layer *overlay);
  void popLayer(Layer *layer);
  void popOverlay(Layer *layer);

  std::vector<Layer *>::iterator begin() { return layers.begin(); }
  std::vector<Layer *>::iterator end() { return layers.end(); }

private:
  std::vector<Layer *> layers;
  std::vector<Layer *>::iterator insert;
};
} // namespace Explorer

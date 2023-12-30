#include <Layer/LayerStack.h>

Explorer::LayerStack::LayerStack() { this->insert = layers.begin(); }

Explorer::LayerStack::~LayerStack() {
  for (Layer *layer : layers)
    delete layer;
}

void Explorer::LayerStack::pushLayer(Layer *layer) { this->layers.emplace(this->insert, layer); }

void Explorer::LayerStack::pushOverlay(Layer* overlay) {
	this->layers.emplace_back(overlay);
}

void Explorer::LayerStack::popLayer(Layer* layer) {
	std::vector<Layer*>::iterator position = std::find(layers.begin(), layers.end(), layer);
	if (position != layers.end()) {
		layers.erase(position);
		insert--;
	}
}


void Explorer::LayerStack::popOverlay(Layer* overlay) {

	std::vector<Layer*>::iterator position = std::find(layers.begin(), layers.end(), overlay);
	if (position != layers.end()) {
		layers.erase(position);
	}

}

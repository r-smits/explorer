#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>
#include <sstream>

Explorer::ViewDelegate::ViewDelegate(MTL::Device *device)
    : MTK::ViewDelegate(), renderer(new Renderer(device)) {

  // Set up Keyboard IO eventing from MTK::View
  ViewAdapter *viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event &event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);
}

void Explorer::ViewDelegate::onEvent(Explorer::Event &event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(onKeyReleased));

  for (std::vector<Layer *>::iterator index = layerStack.end(); index != layerStack.begin();) {
    (*--index)->onEvent(event);
    if (event.isHandled())
      break;
  }
}

bool Explorer::ViewDelegate::onKeyPressed(KeyPressedEvent &event) {
  DEBUG(event.toString());
  return true;
}

bool Explorer::ViewDelegate::onKeyReleased(KeyReleasedEvent &event) {
  DEBUG(event.toString());
  return true;
}

Explorer::ViewDelegate::~ViewDelegate() { delete renderer; }

void Explorer::ViewDelegate::pushLayer(Layer *layer) { this->layerStack.pushLayer(layer); }

void Explorer::ViewDelegate::pushOverlay(Layer *layer) { this->layerStack.pushOverlay(layer); }

void Explorer::ViewDelegate::drawInMTKView(MTK::View *view) {
  renderer->draw(view);
  // ImGui_ImplOSX_NewFrame(view);

  for (Layer *layer : this->layerStack)
    layer->onUpdate();
}

void Explorer::ViewDelegate::drawableSizeWillChange(MTK::View *view, CGSize size) {

  std::stringstream ss;
  ss << "Drawable size event (" << size.height << ", " << size.width << ")";
  DEBUG(ss.str());
}

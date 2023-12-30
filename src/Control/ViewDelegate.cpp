#include "Control/ViewAdapter.hpp"
#include <Control/ViewDelegate.h>
#include <sstream>

Explorer::ViewDelegate::ViewDelegate(MTL::Device *device)
    : MTK::ViewDelegate(), renderer(new Renderer(device)) {
  ViewAdapter *viewAdapter = ViewAdapter::sharedInstance();
  auto callback = [this](Event &event) { this->onEvent(event); };
  viewAdapter->setHandler(callback);
}

void Explorer::ViewDelegate::onEvent(Explorer::Event &event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(onKeyReleased));
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

void Explorer::ViewDelegate::drawInMTKView(MTK::View *view) { renderer->draw(view); }

void Explorer::ViewDelegate::drawableSizeWillChange(MTK::View *view, CGSize size) {

  std::stringstream ss;
  ss << "Drawable size event (" << size.height << ", " << size.width << ")";
  DEBUG(ss.str());
}

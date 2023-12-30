#pragma once
#include <Control/AppProperties.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Events/Events.h>
#include <Events/KeyEvent.h>
#include <functional>
#include <pch.h>
/**
 * 1) AppDelegate -> ViewAdapter -> ViewExtender (C++ -> Obj-C)
 * 2) Objc event method calls C++ Handler. (Obj-C -> C++)
 **/

namespace Explorer {
class ViewAdapter {
public:
  static ViewAdapter *sharedInstance();
  virtual MTK::View *getView(CGRect frame) const;
  virtual void printDebug() const;

  void setHandler(const std::function<void(Event &)> &func) { this->handler = func; }
  void onEvent(Event &event) {
    if (!handler) {
      WARN("No handler was set to handle events.");
      return;
    }
    this->handler(event);
  };

private:
  std::function<void(Event &)> handler;
};

}; // namespace Explorer

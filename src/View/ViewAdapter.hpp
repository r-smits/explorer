#pragma once
#include <pch.h>
/**
 * 1) AppDelegate -> ViewAdapter -> ViewExtender (C++ -> Obj-C)
 * 2) Objc event method calls C++ Handler. (Obj-C -> C++)
 **/

namespace Explorer {
class ViewAdapter {
public:
  static ViewAdapter *sharedInstance();
	static MTK::View* getView();
  static MTK::View *initView(CGRect frame);
	static CGRect bounds();
  virtual void printDebug() const;
  virtual void setHandler(const std::function<void(Event &)> &func);
  virtual void onEvent(Event &event);

private:
  std::function<void(Event &)> handler;
};

}; // namespace Explorer

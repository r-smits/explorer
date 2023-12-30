#pragma once
#include "Events/KeyEvent.h"
#include <Control/AppProperties.h>
#include <Control/ViewAdapter.hpp>
#include <Control/ViewDelegate.h>
#include <CoreFoundation/CFCGTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Events/Events.h>
#include <pch.h>

namespace Explorer {

class AppDelegate : public NS::ApplicationDelegate {

public:
  AppDelegate(Explorer::AppProperties *properties);
  ~AppDelegate();

  virtual void applicationWillFinishLaunching(NS::Notification *msg) override;
  virtual void applicationDidFinishLaunching(NS::Notification *msg) override;
  virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *sender) override;
  virtual double getWidth();
  virtual double getHeight();

private:
  NS::Window *window;
  MTK::View *mtkView;
  MTL::Device *device;
  AppProperties *properties;
  ViewDelegate *viewDelegate = nullptr;
};

} // namespace Explorer

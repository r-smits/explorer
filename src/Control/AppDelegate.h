#pragma once
#include <Control/AppProperties.h>
#include <Events/Events.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>
#include <pch.h>

namespace Explorer {

class AppDelegate : public NS::ApplicationDelegate {

public:
  AppDelegate(Explorer::AppProperties *properties);
  ~AppDelegate();

  virtual void applicationWillFinishLaunching(NS::Notification *msg) override;
  virtual void applicationDidFinishLaunching(NS::Notification *msg) override;
  virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *sender) override;
  virtual void printDebug();
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

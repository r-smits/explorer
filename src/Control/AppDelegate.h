#pragma once
#include <Control/AppProperties.h>
#include <Events/Events.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewDelegate.h>
#include <pch.h>

namespace EXP {

class AppDelegate : public NS::ApplicationDelegate {

public:
  AppDelegate(std::shared_ptr<const EXP::AppProperties> properties);
  ~AppDelegate();

  virtual void applicationWillFinishLaunching(NS::Notification *msg) override;
  virtual void applicationDidFinishLaunching(NS::Notification *msg) override;
  virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application *sender) override;
  virtual void printDebug();
  virtual const double& getWidth();
  virtual const double& getHeight();

private:
  NS::Window *window;
  MTK::View *mtkView;
  MTL::Device *device;
	std::shared_ptr<const EXP::AppProperties> properties;
	EXP::ViewDelegate *viewDelegate = nullptr;
};

} // namespace EXP

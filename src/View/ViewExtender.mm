#include <Events/KeyEvent.h>
#include <Metal/Metal.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewExtender.h>
#include <pch.h>

#include <imgui.h>
#include <imgui_impl_metal.h>
#include <imgui_impl_osx.h>

ViewExtender *extender;
Explorer::ViewAdapter *adapter;

Explorer::ViewAdapter *Explorer::ViewAdapter::sharedInstance() {
  if (!adapter) {
    DEBUG("Initializing new viewAdapter ...");
    adapter = new Explorer::ViewAdapter();
  }
  return adapter;
}

MTK::View *Explorer::ViewAdapter::getView(CGRect frame) const {
  [ViewExtender load:frame];
  return (__bridge MTK::View *)[ViewExtender get];
}

void Explorer::ViewAdapter::printDebug() const {
  ViewExtender *ref = [ViewExtender get];
  [ref printDebug];
}

void Explorer::ViewAdapter::setHandler(
    const std::function<void(Explorer::Event &)> &func) {
  this->handler = func;
}

void Explorer::ViewAdapter::onEvent(Explorer::Event &event) {
  if (!handler) {
    WARN("No handler was set to handle events.");
    return;
  }
  this->handler(event);
}

// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void Explorer::ViewAdapter::imGuiInit(MTL::Device *device) {
  DEBUG("Initializing ImGui ...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init((__bridge id<MTLDevice>)device);
}

@implementation ViewExtender

+ (void)load:(CGRect)frame {
  DEBUG("Initializing Objective-c ViewExtender ...");
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  extender = [[self alloc] initWithFrame:frame];
  [extender init];
  [pool release];
}

+ (ViewExtender *)get {
  return extender;
}

- (id)init {
  BOOL isFirstResponder = [self becomeFirstResponder];
  (isFirstResponder) ? DEBUG("MTKView is now first responder.")
                     : WARN("MTKView is not first responder.");
  return self;
}

// Needs to be overwritten to accept keyboard events.
// Learn more here:
// https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/EventHandlingBasics/EventHandlingBasics.html#//apple_ref/doc/uid/10000060i-CH5
- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)keyDown:(NSEvent *)event {
  NSEventType eventType = [event type];
  if (eventType == NSEventTypeKeyDown) {
    const char *ckey =
        [[event characters] cStringUsingEncoding:NSUTF8StringEncoding];
    const Explorer::KeyPressedEvent event = Explorer::KeyPressedEvent(ckey, 0);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)event);
  } else {
    NSLog(@"[keyDown] Unknown event: %lu", eventType);
  }
}

- (void)keyUp:(NSEvent *)event {
  if ([event type] == NSEventTypeKeyUp) {
    const char *ckey =
        [[event characters] cStringUsingEncoding:NSUTF8StringEncoding];
    const Explorer::KeyReleasedEvent event = Explorer::KeyReleasedEvent(ckey);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)event);
  } else {
    NSLog(@"[keyUp] Unknown event: %lu", [event type]);
  }
}

- (void)printDebug {
  DEBUG("ViewAdapter debug info ...");
}

@end

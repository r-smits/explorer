#include <AppKit/AppKit.h>
#include <AppKit/NSTrackingArea.h>
#include <Metal/Metal.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewExtender.h>
#include <pch.h>

ViewExtender *extender;
Explorer::ViewAdapter *adapter;

Explorer::ViewAdapter *Explorer::ViewAdapter::sharedInstance() {
  if (!adapter) {
    DEBUG("Initializing new viewAdapter ...");
    adapter = new Explorer::ViewAdapter();
  }
  return adapter;
}

MTK::View *Explorer::ViewAdapter::initView(CGRect frame) {
  [ViewExtender load:frame];
  return (__bridge MTK::View *)[ViewExtender get];
}

MTK::View *Explorer::ViewAdapter::getView() {
  if (!extender) {
    WARN("Atempted to retrieve MTK::View but not initialized!");
  }
  return (__bridge MTK::View *)[ViewExtender get];
}

CGRect Explorer::ViewAdapter::bounds() {
if (!extender) {
	WARN("Attemtped to retrieve MTK::View bounds, but not initialized!");
}
	return (CGRect) [ViewExtender get].bounds;
}

void Explorer::ViewAdapter::printDebug() const {
  ViewExtender *ref = [ViewExtender get];
  [ref printDebug];
}

void Explorer::ViewAdapter::setHandler(const std::function<void(Explorer::Event &)> &func) {
  this->handler = func;
}

void Explorer::ViewAdapter::onEvent(Explorer::Event& event) {
  if (!handler) {
    WARN("No handler was set to handle events.");
    return;
  }
  this->handler(event);
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

- (BOOL)acceptsFirstMouse:(NSEvent *)event {
  return YES;
}

- (void)updateTrackingAreas {
  DEBUG("Update tracking areas ...");
  NSTrackingArea *areaInit =
      [[NSTrackingArea alloc] initWithRect:[self bounds]
                                   options:(NSTrackingMouseMoved | NSTrackingActiveAlways)
                                     owner:self
                                  userInfo:nil];
  [self addTrackingArea:areaInit];
}

- (void)mouseUp:(NSEvent *)event {
  if ([event type] == NSEventType::NSEventTypeLeftMouseUp || NSEventType::NSEventTypeRightMouseUp) {
    Explorer::MouseButtonReleasedEvent released =
        Explorer::MouseButtonReleasedEvent([event buttonNumber]);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)released);
  } else {
    NSLog(@"MouseUp :: Unknown event: %lu", [event type]);
  }
}

- (void)mouseDown:(NSEvent *)event {
  if ([event type] == NSEventType::NSEventTypeLeftMouseDown ||
      NSEventType::NSEventTypeRightMouseDown) {
    Explorer::MouseButtonPressedEvent pressed =
        Explorer::MouseButtonPressedEvent([event buttonNumber]);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)pressed);
  } else {
    NSLog(@"MouseDown :: Unknown event: %lu", [event type]);
  }
}

- (void)mouseMoved:(NSEvent *)event {
  if ([event type] == NSEventType::NSEventTypeMouseMoved) {
    NSPoint mousePoint = event.locationInWindow;
    //mousePoint = [self convertPoint:mou:ePoint fromView:nil];
		
			
    mousePoint = NSMakePoint(mousePoint.x*2, (self.bounds.size.height - mousePoint.y)*2);
    Explorer::MouseMoveEvent moved = Explorer::MouseMoveEvent(mousePoint.x, mousePoint.y);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)moved);
  } else {
    NSLog(@"MouseMoved :: Unknown event: %lu", [event type]);
  }
}

- (void)mouseDragged:(NSEvent *)event {
  if ([event type] == NSEventType::NSEventTypeLeftMouseDragged ||
      NSEventType::NSEventTypeRightMouseDragged) {
    NSPoint mousePoint = event.locationInWindow;
    mousePoint = [self convertPoint:mousePoint fromView:nil];
    mousePoint = NSMakePoint(mousePoint.x, self.bounds.size.height - mousePoint.y);
    Explorer::MouseMoveEvent moved = Explorer::MouseMoveEvent(mousePoint.x, mousePoint.y);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)moved);

  } else {
    NSLog(@"MouseDragged :: Unknown event: %lu", [event type]);
  }
}

- (void)mouseExited:(NSEvent *)event {
  [event type];
  std::stringstream ss;
  ss << [event type] << "!";
  DEBUG(ss.str());
}

- (void)mouseEntered:(NSEvent *)event {
  [event type];
  std::stringstream ss;
  ss << [event type] << "!";
  DEBUG(ss.str());
}

- (void)keyDown:(NSEvent *)event {

  if ([event type] == NSEventTypeKeyDown) {
    const Explorer::KeyPressedEvent pressed =
        Explorer::KeyPressedEvent([event keyCode], [event isARepeat]);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)pressed);
  } else {
    NSLog(@"KeyDown :: Unknown event: %lu", [event type]);
  }
}

- (void)keyUp:(NSEvent *)event {
  if ([event type] == NSEventTypeKeyUp) {
    const Explorer::KeyReleasedEvent pressed = Explorer::KeyReleasedEvent([event keyCode]);
    Explorer::ViewAdapter::sharedInstance()->onEvent((Explorer::Event &)pressed);
  } else {
    NSLog(@"KeyUp :: Unknown event: %lu", [event type]);
  }
}

- (void)printDebug {
  DEBUG("ViewAdapter debug info ...");
}

@end

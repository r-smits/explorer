#include "Events/KeyEvent.h"
#include "Foundation/NSString.hpp"
#include <AppKit/AppKit.h>
#include <Control/AppProperties.h>
#include <Control/ViewAdapter.hpp>
#include <Control/ViewExtender.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <Foundation/NSString.h>
#include <cstdio>
#include <pch.h>
#include <string>

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
  NSLog(@"Is first responder: %@", isFirstResponder ? @"Yes" : @"No");
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

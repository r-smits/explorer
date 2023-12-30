#pragma once
#include <AppKit/AppKit.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <GameController/GCKeyboard.h>
#include <GameController/GameController.h>
#include <MetalKit/MTKView.h>
#include <iostream>
#include <objc/NSObject.h>

/**
 * 1) AppDelegate -> KeyboardAdapterCXX (C++ -> Obj-C -> C++)
 * 2) Handler calls C++ handler. (Obj-C -> C++)
 **/

@interface ViewExtender : MTKView {
}
+ (void)load:(CGRect)frame;
+ (ViewExtender *)get;
- (void)printDebug;
- (void)keyDown:(NSEvent *)event;
- (void)keyUp:(NSEvent *)event;
@end

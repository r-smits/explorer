#pragma once
#include <MetalKit/MTKView.h>
#include <pch.h>

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

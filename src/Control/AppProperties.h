#pragma once
#include <pch.h>

namespace Explorer {

class AppProperties {

  /**
   * GCRect looks like the following
   *
   * struct CGRect {
   *		CGPoint origin;
   *		CGSize size;
   * };
   * typedef struct CF_BOXABLE CGRect CGRect;
   **/

public:
  AppProperties(double width, double height) { cgRect = (CGRect){{100, 100}, {width, height}}; }
  ~AppProperties() {}
  CGRect cgRect;
};
}; // namespace Explorer

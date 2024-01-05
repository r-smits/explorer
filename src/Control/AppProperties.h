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
  AppProperties(double width, double height, std::string shaderPath, std::string texturePath)
      : shaderPath(shaderPath), texturePath(texturePath), cgRect({
                                                              {100.0f, 100.0f},
                                                              { width, height}
  }) {}
  ~AppProperties() {}
  CGRect cgRect;
  const std::string shaderPath;
  const std::string texturePath;
  std::string vertexFnName;
  std::string fragmentFnName;
};
}; // namespace Explorer

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
  AppProperties(
      double width,
      double height,
      std::string shaderPath,
      std::string texturePath,
      std::string meshPath
  )
      : shaderPath(shaderPath), texturePath(texturePath), meshPath(meshPath),
        cgRect({
            {100.0f, 100.0f},
            { width, height}
  }) {}
  ~AppProperties() {}
  CGRect cgRect;
  const std::string shaderPath;
  const std::string texturePath;
  const std::string meshPath;
  std::string vertexFnName;
  std::string fragmentFnName;
};
}; // namespace Explorer

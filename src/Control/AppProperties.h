#pragma once
#include <pch.h>


using path = std::filesystem::path;

namespace EXP {

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
      const path& shader_path,
      const path& texture_path,
      const path& mesh_path
  )
      : shader_path(shader_path), texture_path(texture_path), mesh_path(mesh_path),
        cgRect({
            {100.0f, 100.0f},
            { width, height}
  }) {}
  ~AppProperties() {}
  CGRect cgRect;
  const path shader_path;
  const path texture_path;
  const path mesh_path;
  std::string vertexFnName;
  std::string fragmentFnName;
};
}; // namespace EXP

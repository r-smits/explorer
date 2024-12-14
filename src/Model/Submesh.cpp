#include <Model/Submesh.h>

const void EXP::MDL::Submesh::setColor(const simd::float4& color) {
  Renderer::PrimitiveAttributes* primAttribPtr =
      (Renderer::PrimitiveAttributes*)primitiveBuffer->contents();
  for (int i = 0; i < indexCount / 3; i += 1) {
    (primAttribPtr + i)->color[0] = color;
    (primAttribPtr + i)->color[1] = color;
    (primAttribPtr + i)->color[2] = color;
  }
}

const void EXP::MDL::Submesh::setEmissive(const bool& emissive) {
  Renderer::PrimitiveAttributes* primAttribPtr =
      (Renderer::PrimitiveAttributes*)primitiveBuffer->contents();
  for (int i = 0; i < indexCount / 3; i += 1) {
    (primAttribPtr + i)->flags[1] = emissive;
  }
}

#pragma once
#include <Renderer/Types.h>
#include <pch.h>

namespace EXP {

namespace MDL {

struct Submesh {

  MTL::Buffer* indexBuffer;
  MTL::Buffer* primitiveBuffer;

  MTL::PrimitiveType primitiveType;
  MTL::IndexType indexType;

  NS::UInteger indexCount;
  NS::UInteger offset;

  Submesh(
			MTL::Buffer* indexBuffer,
      MTL::Buffer* primitiveBuffer,
      const MTL::PrimitiveType& primitiveType,
      const MTL::IndexType& indexType,
			const int& indexCount,
      const int& offset
  )
      : primitiveType(primitiveType), indexCount(indexCount), indexType(indexType),
        indexBuffer(indexBuffer), primitiveBuffer(primitiveBuffer), offset(offset) {}

  ~Submesh() {
    indexBuffer->release();
    primitiveBuffer->release();
  }

  const void setColor(const simd::float4& color) {
    Renderer::PrimitiveAttributes* primAttribPtr =
        (Renderer::PrimitiveAttributes*)primitiveBuffer->contents();
    for (int i = 0; i < indexCount / 3; i += 1) {
      (primAttribPtr + i)->color[0] = color;
      (primAttribPtr + i)->color[1] = color;
      (primAttribPtr + i)->color[2] = color;
    }
  }

  const void setEmissive(const bool& emissive) {
    Renderer::PrimitiveAttributes* primAttribPtr =
        (Renderer::PrimitiveAttributes*)primitiveBuffer->contents();
    for (int i = 0; i < indexCount / 3; i += 1) {
      (primAttribPtr + i)->flags[1] = emissive;
    }
  }
};
}; // namespace MDL
}; // namespace EXP

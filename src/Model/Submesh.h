#pragma once
#include <Renderer/Types.h>
#include <pch.h>

namespace EXP {

namespace MDL {

struct Submesh {

  Renderer::Material material;
  std::vector<MTL::Texture*> textures;
  MTL::PrimitiveType primitiveType;
  NS::UInteger indexCount;
  MTL::IndexType indexType;
  MTL::Buffer* indexBuffer;
  MTL::Buffer* primitiveBuffer;
  NS::UInteger offset;
  NS::UInteger texindex;

  Submesh(
      const Renderer::Material& material,
      const std::vector<MTL::Texture*>& textures,
      const MTL::PrimitiveType& primitiveType,
      const int& indexCount,
      const MTL::IndexType& indexType,
      MTL::Buffer* indexBuffer,
      MTL::Buffer* primitiveBuffer,
      const int& offset
  )
      : material(material), textures(textures), primitiveType(primitiveType),
        indexCount(indexCount), indexType(indexType), indexBuffer(indexBuffer),
        primitiveBuffer(primitiveBuffer), offset(offset) {}

  ~Submesh() {
    indexBuffer->release();
    primitiveBuffer->release();
    for (MTL::Texture* texture : textures)
      texture->release();
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
};
}; // namespace MOD
}; // namespace EXP

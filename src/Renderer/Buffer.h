#pragma once
#include <Renderer/Types.h>
#include <cstdint>
#include <pch.h>

namespace Renderer {


class Buffer {
public:
  virtual ~Buffer();
  virtual void bind() = 0;
  virtual void unbind() = 0;

  static MTL::Buffer* create(MTL::Device* device, Vertex vertices[], uint32_t size);
  static MTL::Buffer* create(MTL::Device* device, ushort indices[], uint32_t size);
  static MTL::Buffer* create(MTL::Device* device, Light* light);
	
	static MTL::Buffer* perPrimitive(
			MTL::Device* device, 
			MTL::Buffer* vertexAttribBuffer,
			MTL::Buffer* indices,
			const int& indexCount,
			const int& texindex
	);

	static void setVertexColor(
			MTL::Buffer* vertexAttribBuffer,
			MTL::Buffer* indices,
			const int& indexCount,
			const simd::float4& color
	);
};

}; // namespace EXP

#pragma once
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Renderer {

class Descriptor {
public:
  static MTL::VertexDescriptor* vertex(MTL::Device* device, BufferLayout* layout);
	static MTL::DepthStencilState* getDepthStencilState();
  virtual MTL::SamplerState* getSamplerState();


};
}; // namespace Renderer

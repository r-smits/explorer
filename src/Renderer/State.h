#pragma once
#include <pch.h>

namespace Renderer {
struct State {

public:
  static MTL::RenderPipelineState*
  render(MTL::Device* device, MTL::RenderPipelineDescriptor* descriptor);
  static MTL::ComputePipelineState* compute(MTL::Device* device, std::string path);
	static MTL::ComputePipelineState* Compute(MTL::Device* device, MTL::Function* fn);
  static MTL::DepthStencilState* depthStencil(MTL::Device* device);
  static MTL::SamplerState* sampler(MTL::Device* device);
};
}; // namespace Renderer

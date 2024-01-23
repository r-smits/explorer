#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Renderer {

typedef std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> PDV;

struct Descriptor {
public:
  static MTL::VertexDescriptor* vertex(MTL::Device* device, const BufferLayouts& layouts);
  static MTL::RenderPipelineDescriptor*
  render(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, std::string path);
  static MTL::ComputePipelineDescriptor* compute(MTL::Device* device, std::string path);
  static MTL::PrimitiveAccelerationStructureDescriptor* primitive(Explorer::Mesh* mesh);
  static PDV primitives(Explorer::Model* model);
};
}; // namespace Renderer

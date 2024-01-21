#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Renderer {

typedef std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> PDV;
typedef std::vector<MTL::AccelerationStructure*> AccelerationStructures;

struct Descriptor {
public:
  static MTL::VertexDescriptor* vertex(MTL::Device* device, const BufferLayouts& layouts);
	static MTL::RenderPipelineDescriptor* render(MTL::Device* device, const std::string& path);
	static MTL::ComputePipelineDescriptor* compute(MTL::Device* device, const std::string& path);
  static MTL::PrimitiveAccelerationStructureDescriptor* primitive(const Explorer::Mesh& mesh);
  static PDV primitives(Explorer::Model* model);
	static MTL::InstanceAccelerationStructureDescriptor* instance(
		MTL::Device* device,
		NS::Array* structures,
		const int& instanceCount
			);

	//static NS::Array* fromVector(std::vector<T> v);
};
}; // namespace Renderer

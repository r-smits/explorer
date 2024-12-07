#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Renderer {

typedef std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> PDV;

struct Descriptor {
  
	static MTL::VertexDescriptor* vertex(
			MTL::Device* device, 
			const BufferLayouts& layouts
	);

  static MTL::RenderPipelineDescriptor* render(
			MTL::Device* device, 
			MTL::VertexDescriptor* vertexDescriptor, 
			const std::string& path
	);

  static MTL::ComputePipelineDescriptor* compute(
			MTL::Device* device, 
			const std::string& path
	);

  static MTL::PrimitiveAccelerationStructureDescriptor* primitive(
			EXP::MDL::Mesh* mesh, 
			const int& vStride, 
			const int& pStride
	);

  static std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> primitives(
			const std::vector<EXP::Model*>& scene, 
			const int& vStride, 
			const int& pStride
	);

  static MTL::InstanceAccelerationStructureDescriptor* instance(
      MTL::Device* device,
      const std::vector<MTL::AccelerationStructure*>& primitiveStructures,
      const std::vector<EXP::Model*>& scene
  );

	static MTL::InstanceAccelerationStructureDescriptor* updateTransformationMatrix(
			const std::vector<EXP::Model*>& scene,
			MTL::InstanceAccelerationStructureDescriptor* descriptor
	);

};
}; // namespace Renderer

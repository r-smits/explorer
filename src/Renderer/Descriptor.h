#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

using inst_desc = MTL::AccelerationStructureInstanceDescriptor;
using geom_desc = MTL::AccelerationStructureTriangleGeometryDescriptor;
using inst_acc_desc = MTL::InstanceAccelerationStructureDescriptor;
using prim_acc_desc = MTL::PrimitiveAccelerationStructureDescriptor;

using mesh_array = const std::vector<EXP::MDL::Mesh*>&;
using acc_array = const std::vector<MTL::AccelerationStructure*>&;


namespace Renderer {

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

	static std::vector<prim_acc_desc*> primitives(
		mesh_array meshes, 
		const int& vStride, 
		const int& pStride
	);

	static prim_acc_desc* primitive(
		EXP::MDL::Mesh* mesh, 
		const int& vStride, 
		const int& pStride
	);

	static inst_acc_desc* instance(
		MTL::Device* device,
		acc_array primitiveStructures,
		mesh_array meshes
	);

	static inst_acc_desc* updateTransformationMatrix(
		mesh_array meshes, 
		inst_acc_desc* descriptor
	);

};
}; // namespace Renderer

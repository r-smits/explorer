#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLDevice.hpp"
#include <pch.h>


using prim_acc_desc = MTL::PrimitiveAccelerationStructureDescriptor;
using prim_acc_desc_array = const std::vector<prim_acc_desc*>&;

namespace Renderer {

struct Acceleration {

  	static MTL::AccelerationStructureSizes sizes(
		MTL::Device* device, 
		prim_acc_desc_array descriptors
	);

  static std::vector<MTL::AccelerationStructure*> primitives(
		MTL::Device* device,
		MTL::Heap* heap,
		MTL::CommandQueue* queue,
		prim_acc_desc_array descriptors,
		const MTL::AccelerationStructureSizes& sizes,
		MTL::Event* buildEvent
	);

	static NS::Array* primitivesWithoutHeapAllocation(
		MTL::Device* device,
		MTL::CommandQueue* queue,
		NS::Array* descriptors,
		NS::Array* heapStructures,
		const MTL::AccelerationStructureSizes& sizes,
		MTL::Event* buildEvent
	);

	static MTL::AccelerationStructure* instance(
		MTL::Device* device,
		MTL::CommandQueue* queue,
		MTL::AccelerationStructure* structure,
		MTL::InstanceAccelerationStructureDescriptor* descriptor,
		MTL::Buffer* scratch_buffer,
		MTL::Event* buildEvent
	);

	static MTL::AccelerationStructure* refit(
		MTL::Device* device,
		MTL::CommandQueue* queue,
		MTL::InstanceAccelerationStructureDescriptor* descriptor,
		MTL::AccelerationStructure* structure,
		MTL::Event* buildEvent
	);
};
}; // namespace Renderer

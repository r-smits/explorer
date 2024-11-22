#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLDevice.hpp"
#include <pch.h>

namespace Renderer {

struct Acceleration {

  static MTL::AccelerationStructureSizes sizes(
			MTL::Device* device, 
			const std::vector<MTL::PrimitiveAccelerationStructureDescriptor*>& descriptors
	);

  static std::vector<MTL::AccelerationStructure*> primitives(
      MTL::Device* device,
      MTL::Heap* heap,
      MTL::CommandQueue* queue,
      const std::vector<MTL::PrimitiveAccelerationStructureDescriptor*>& descriptors,
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
      MTL::InstanceAccelerationStructureDescriptor* descriptor,
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

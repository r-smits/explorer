#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLDevice.hpp"
#include <Renderer/Heap.h>

// You need to calculate the size of acceleration structure and returns heap
// with size
MTL::Heap* Renderer::Heap::primitives(MTL::Device* device, MTL::AccelerationStructureSizes sizes) {
  
	/**
	MTL::AccelerationStructureSizes totalSizes = {0, 0, 0};

  for (int i = 0; i < descriptors->count(); i++) {
    MTL::PrimitiveAccelerationStructureDescriptor* descriptor =
        (MTL::PrimitiveAccelerationStructureDescriptor*)descriptors->object(i);
    MTL::SizeAndAlign sizeAlign = device->heapAccelerationStructureSizeAndAlign(descriptor);
    MTL::AccelerationStructureSizes sizes = device->accelerationStructureSizes(descriptor);
    totalSizes.accelerationStructureSize += (sizeAlign.size + sizeAlign.align);
    totalSizes.buildScratchBufferSize =
        std::max(sizes.buildScratchBufferSize, totalSizes.buildScratchBufferSize);
    totalSizes.refitScratchBufferSize =
        std::max(sizes.refitScratchBufferSize, totalSizes.refitScratchBufferSize);
  }
	**/

  MTL::HeapDescriptor* heapDescriptor = MTL::HeapDescriptor::alloc()->init();
  heapDescriptor->setSize(sizes.accelerationStructureSize);
  return device->newHeap(heapDescriptor);
}




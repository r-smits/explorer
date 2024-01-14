#include <Renderer/Heap.h>

// You need to calculate the size of acceleration structure and returns heap with size
MTL::Heap* Renderer::Heap::fromPrimitives(MTL::Device* device, PDV dPrimitives) {
  
  MTL::AccelerationStructureSizes tSizes = {0, 0, 0};

  for (MTL::PrimitiveAccelerationStructureDescriptor* dPrimitive : dPrimitives) {
    MTL::SizeAndAlign sizeAlign = device->heapAccelerationStructureSizeAndAlign(dPrimitive);
    MTL::AccelerationStructureSizes sizes = device->accelerationStructureSizes(dPrimitive);
    tSizes.accelerationStructureSize += (sizeAlign.size + sizeAlign.align);
    tSizes.buildScratchBufferSize =
        std::max(sizes.buildScratchBufferSize, tSizes.buildScratchBufferSize);
    tSizes.refitScratchBufferSize =
        std::max(sizes.refitScratchBufferSize, tSizes.refitScratchBufferSize);
  }
	
	MTL::HeapDescriptor* dHeap = MTL::HeapDescriptor::alloc()->init();
	dHeap->setSize(tSizes.accelerationStructureSize);
  return device->newHeap(dHeap);
}





#include <Renderer/Heap.h>

// You need to calculate the size of acceleration structure and returns heap
// with size
MTL::Heap* Renderer::Heap::primitives(MTL::Device* device, MTL::AccelerationStructureSizes sizes) {
  MTL::HeapDescriptor* heapDescriptor = MTL::HeapDescriptor::alloc()->init();
  heapDescriptor->setSize(sizes.accelerationStructureSize);
  return device->newHeap(heapDescriptor);
}




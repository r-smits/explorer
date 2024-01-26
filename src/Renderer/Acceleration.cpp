#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLAccelerationStructureCommandEncoder.hpp"
#include "Metal/MTLDevice.hpp"
#include <Renderer/Acceleration.h>

MTL::AccelerationStructureSizes
Renderer::Acceleration::sizes(MTL::Device* device, NS::Array* descriptors) {
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
  return totalSizes;
}

NS::Array* Renderer::Acceleration::primitives(
    MTL::Device* device,
    MTL::Heap* heap,
    MTL::CommandQueue* queue,
    NS::Array* descriptors,
    const MTL::AccelerationStructureSizes& sizes,
    MTL::Event* buildEvent
) {
  std::vector<MTL::AccelerationStructure*> structures;
  MTL::Buffer* scratchBuffer =
      device->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate);
  MTL::CommandBuffer* cmd = queue->commandBuffer();
  MTL::AccelerationStructureCommandEncoder* encoder = cmd->accelerationStructureCommandEncoder();
  for (int i = 0; i < descriptors->count(); i++) {
    MTL::PrimitiveAccelerationStructureDescriptor* descriptor =
        (MTL::PrimitiveAccelerationStructureDescriptor*)descriptors->object(i);
    MTL::SizeAndAlign sizeAlign = device->heapAccelerationStructureSizeAndAlign(descriptor);
    MTL::AccelerationStructure* structure = heap->newAccelerationStructure(sizeAlign.size);
    encoder->buildAccelerationStructure(structure, descriptor, scratchBuffer, 0);
    structures.emplace_back(structure);
  }
  encoder->endEncoding();
  cmd->encodeSignalEvent(buildEvent, 1);
  cmd->commit();
  NS::Array* result = NS::Array::array((NS::Object* const*)&structures[0], structures.size());
  DEBUG(
      "Built " + std::to_string(descriptors->count()) + " primitive acceleration structures on heap."
  );
  return result;
}

MTL::AccelerationStructure* Renderer::Acceleration::instance(
    MTL::Device* device,
    MTL::CommandQueue* queue,
    MTL::InstanceAccelerationStructureDescriptor* descriptor,
    MTL::Event* buildEvent
) {
  MTL::CommandBuffer* cmd = queue->commandBuffer();
  cmd->encodeWait(buildEvent, 1);
  MTL::AccelerationStructureSizes sizes = device->accelerationStructureSizes(descriptor);
  MTL::Buffer* scratchBuffer =
      device->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate);
  MTL::AccelerationStructure* structure =
      device->newAccelerationStructure(sizes.accelerationStructureSize);
  MTL::AccelerationStructureCommandEncoder* encoder = cmd->accelerationStructureCommandEncoder();
  encoder->buildAccelerationStructure(structure, descriptor, scratchBuffer, NS::UInteger(0));
  encoder->endEncoding();
  cmd->encodeSignalEvent(buildEvent, 2);
  cmd->commit();
	DEBUG("Built instance acceleration structure.");
  return structure;
}


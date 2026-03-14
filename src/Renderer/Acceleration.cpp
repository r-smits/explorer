#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLAccelerationStructureCommandEncoder.hpp"
#include <Metal/MTLDevice.hpp>
#include <Renderer/Acceleration.h>

MTL::AccelerationStructureSizes Renderer::Acceleration::sizes(
		MTL::Device* device,
		prim_acc_desc_array descriptors
) {
  MTL::AccelerationStructureSizes totalSizes = {0, 0, 0};
  for (int i = 0; i < descriptors.size(); i++) {
    MTL::PrimitiveAccelerationStructureDescriptor* descriptor = descriptors[i];
    MTL::SizeAndAlign sizeAlign = device->heapAccelerationStructureSizeAndAlign(descriptor);
    MTL::AccelerationStructureSizes sizes = device->accelerationStructureSizes(descriptor);
    totalSizes.accelerationStructureSize += (sizeAlign.size + sizeAlign.align);
    totalSizes.buildScratchBufferSize = std::max(sizes.buildScratchBufferSize, totalSizes.buildScratchBufferSize);
    totalSizes.refitScratchBufferSize = std::max(sizes.refitScratchBufferSize, totalSizes.refitScratchBufferSize);
  }
  return totalSizes;
}

std::vector<MTL::AccelerationStructure*> Renderer::Acceleration::primitives(
    MTL::Device* device,
    MTL::Heap* heap,
    MTL::CommandQueue* queue,
    prim_acc_desc_array descriptors,
    const MTL::AccelerationStructureSizes& sizes,
    MTL::Event* buildEvent
) {
	std::vector<MTL::AccelerationStructure*> structures;
  MTL::Buffer* scratchBuffer = device->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate);
  MTL::CommandBuffer* cmd = queue->commandBuffer();
  MTL::AccelerationStructureCommandEncoder* encoder = cmd->accelerationStructureCommandEncoder();
  for (int i = 0; i < descriptors.size(); i++) {
    MTL::PrimitiveAccelerationStructureDescriptor* descriptor = descriptors[i];
    MTL::SizeAndAlign sizeAlign = device->heapAccelerationStructureSizeAndAlign(descriptor);
    MTL::AccelerationStructure* structure = heap->newAccelerationStructure(sizeAlign.size);
    encoder->buildAccelerationStructure(structure, descriptor, scratchBuffer, 0);
    structures.emplace_back(structure);
	}
  encoder->endEncoding();
  cmd->encodeSignalEvent(buildEvent, 1);
  cmd->commit();
	DEBUG("Allocated heap for primitives. Usage in kb: " + std::to_string(heap->usedSize() / 1024));
	return structures;
}

MTL::AccelerationStructure* Renderer::Acceleration::instance(
  MTL::Device* device,
  MTL::CommandQueue* queue,
  MTL::AccelerationStructure* structure,
  MTL::InstanceAccelerationStructureDescriptor* descriptor,
  MTL::Buffer* scratch_buffer,
  MTL::Event* buildEvent
) {
  MTL::CommandBuffer* cmd = queue->commandBuffer();
  cmd->encodeWait(buildEvent, 1);
  MTL::AccelerationStructureCommandEncoder* encoder = cmd->accelerationStructureCommandEncoder();
  encoder->buildAccelerationStructure(structure, descriptor, scratch_buffer, NS::UInteger(0));
  encoder->endEncoding();
  cmd->encodeSignalEvent(buildEvent, 2);  // signal render pass
  cmd->addCompletedHandler([scratch_buffer](MTL::CommandBuffer*) {});
  cmd->commit();
  return structure;
}


MTL::AccelerationStructure* Renderer::Acceleration::refit(
		MTL::Device* device,
		MTL::CommandQueue* queue,
		MTL::InstanceAccelerationStructureDescriptor* descriptor,
		MTL::AccelerationStructure* structure,
		MTL::Event* buildEvent
) {
  MTL::CommandBuffer* cmd = queue->commandBuffer();
  MTL::AccelerationStructureSizes sizes = device->accelerationStructureSizes(descriptor);
  MTL::Buffer* scratchBuffer = device->newBuffer(sizes.refitScratchBufferSize, MTL::ResourceStorageModePrivate);
  MTL::AccelerationStructureCommandEncoder* encoder = cmd->accelerationStructureCommandEncoder();
  encoder->refitAccelerationStructure(structure, descriptor, nullptr, scratchBuffer, 0);
  encoder->endEncoding();
  cmd->commit();
  cmd->waitUntilCompleted();
  scratchBuffer->release();
  return structure;
}


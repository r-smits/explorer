#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLDevice.hpp"
#include <pch.h>

namespace Renderer {

struct Acceleration {

  static MTL::AccelerationStructureSizes sizes(MTL::Device* device, NS::Array* descriptors);

  static NS::Array* primitives(
      MTL::Device* device,
      MTL::Heap* heap,
      MTL::CommandQueue* queue,
      NS::Array* descriptors,
      const MTL::AccelerationStructureSizes& sizes,
      MTL::Event* buildEvent
  );

  static MTL::AccelerationStructure* instance(
      MTL::Device* device,
      MTL::CommandQueue* queue,
      MTL::InstanceAccelerationStructureDescriptor* descriptor,
      MTL::Event* buildEvent
  );
};
}; // namespace Renderer

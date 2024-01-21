#pragma once
#include <Renderer/Descriptor.h>
#include <pch.h>

namespace Renderer {

struct Accelerator {

  static NS::Array* build(
      MTL::Device* device,
      MTL::CommandQueue queue,
      PDV descriptors,
      MTL::Heap* heap,
      size_t maxScratchBufferSize
  );
};

}; // namespace Renderer

#pragma once
#include "Metal/MTLDevice.hpp"
#include <Renderer/Descriptor.h>
#include <pch.h>

namespace Renderer {

	struct Heap {
		static MTL::Heap* primitives(MTL::Device* device, MTL::AccelerationStructureSizes sizes);
	};
};

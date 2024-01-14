#pragma once
#include <Renderer/Descriptor.h>
#include <pch.h>

namespace Renderer {

	struct Heap {
		static MTL::Heap* fromPrimitives(MTL::Device* device, PDV dPrimitives);


	};
};

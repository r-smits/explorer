#pragma once
#include <pch.h>


namespace Renderer {
	class State {
		State() {};
		~State() {};
		public:
	static MTL::DepthStencilState* depthStencil(MTL::Device* device);
  static MTL::SamplerState* sampler(MTL::Device* device);
	};
};

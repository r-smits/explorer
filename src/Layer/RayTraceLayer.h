#pragma once
#include "Renderer/Types.h"
#include <Layer/Layer.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>
#include <pch.h>

namespace Explorer {

class RayTraceLayer : public Layer {

public: // Setting up layer
  RayTraceLayer(MTL::Device* device, AppProperties* config);
  ~RayTraceLayer() {
		device->release();
		commandQueue->release();
	};

public:  // Event
private: // Initialization
  virtual void onUpdate(MTK::View* view, MTL::CommandBuffer* buffer) override;

private:
  MTL::Device* device;
  MTL::CommandQueue* commandQueue;
  MTL::ComputePipelineState* _raytrace;
	MTL::RenderPipelineState* _render;

private:
  VCamera _camera;
  MTL::CommandQueue* queue;

	MTL::Size _threadGroupSize;
	MTL::Size _gridSize;
	simd::float3 _resolution;

private:
	simd::float3 _lightDir;

	Renderer::Sphere spheres[3];
	Renderer::RTMaterial materials[3];
	Explorer::Model* f16;
};
}; // namespace Explorer

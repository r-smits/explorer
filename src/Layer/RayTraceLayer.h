#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
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
    queue->release();
  };

public: // Event
	void buildModels();
  void buildAccelerationStructures();

private: // Initialization
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) override;

private:
  MTL::Device* device;
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
  std::vector<Explorer::Model*> models;

private:
	MTL::Heap* _heap;
  NS::Array* _primitiveAccStructures;
	MTL::AccelerationStructure* _instanceAccStructure;
  MTL::Event* buildEvent;
};
}; // namespace Explorer

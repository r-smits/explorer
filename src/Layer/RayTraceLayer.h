#pragma once
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLVertexDescriptor.hpp"
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
  void buildModels(MTL::Device* device);
  void buildAccelerationStructures(MTL::Device* device);
	void rebuildAccelerationStructures(MTL::Device* device);
	void buildBindlessScene(MTL::Device* device, const std::vector<Model*>& scene);
	MTL::Size calcGridsize();

private: // Initialization
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) override;

private:
  MTL::Device* device;
	MTL::Function* _kernelFn;
  MTL::ComputePipelineState* _raytrace;
  MTL::RenderPipelineState* _render;

private:
  VCamera _camera;
  MTL::CommandQueue* queue;

  MTL::Size _threadGroupSize;
  MTL::Size _gridSize;
  simd::float3 _resolution;

private:
	MTL::VertexDescriptor* _vertexDescriptor;
  simd::float3 _lightDir;
  Renderer::Sphere _spheres[3];
  Renderer::RTMaterial _materials[3];
  std::vector<Explorer::Model*> scene;
  Explorer::Model* _modelsarr[1];

private:
	MTL::Event* _buildEvent;
  MTL::Heap* _heap;
	std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> _primitiveDescriptors;
	std::vector<MTL::AccelerationStructure*> _primitiveAccStructures;
	MTL::InstanceAccelerationStructureDescriptor* _instanceDescriptor;
  MTL::AccelerationStructure* _instanceAccStructure;
	MTL::Buffer* _sceneBuffer;
	std::vector<MTL::Resource*> _resources;

private:
	int t = 0;


};
}; // namespace Explorer

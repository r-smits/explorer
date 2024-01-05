#pragma once
#include <Shaders/ShaderRepository.h>
#include <Layer/Layer.h>
#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Model/Camera.h>
#include <pch.h>

namespace Explorer {
class BaseLayer : public Layer {

public: // Setting up layer
  BaseLayer(MTL::Device* device);
  ~BaseLayer();

public: // Event
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) override;
  virtual void onEvent(Event& event) override;
  virtual bool onKeyPressed(KeyPressedEvent& event);
  virtual bool onKeyReleased(KeyReleasedEvent& event);
  virtual bool onMouseButtonPressed(MouseButtonPressedEvent& event);
  virtual bool onMouseButtonReleased(MouseButtonReleasedEvent& event);
  virtual bool onMouseMove(MouseMoveEvent& event);
	virtual void checkIO();

private: // Initialization
  virtual void buildPipeline();
  virtual void buildMeshes();
  virtual MTL::RenderPipelineDescriptor* getRenderPipelineDescriptor(std::string shaderName);
  virtual MTL::VertexDescriptor* getVertexDescriptor(BufferLayout* layout);
  virtual MTL::RenderPipelineState* getRenderPipelineState(std::string shaderName, bool serialize);
  virtual MTL::DepthStencilState* getDepthStencilState();

private: // Rendering
	virtual void drawLight(MTL::RenderCommandEncoder* encoder, LightSource* light);
  virtual void drawMesh(MTL::RenderCommandEncoder* encoder, Mesh* mesh);

private:
  ShaderRepository repository;
  MTL::Device* device;
  MTL::CommandQueue* commandQueue;
  MTL::RenderPipelineState* generalPipeline;
  MTL::DepthStencilState* depthStencilState;
  simd::float4x4 projection;

private:
	simd::float4x4 viewMatrix;
	Camera camera;
	LightSource* light;
  MTL::Buffer* mesh;
  Mesh* quadMesh;
  Mesh* pyramid;
  Mesh* cube;
  float t;
};
}; // namespace Explorer

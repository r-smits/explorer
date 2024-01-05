#pragma once
#include <Control/AppProperties.h>
#include <DB/Repository.h>
#include <Layer/Layer.h>
#include <Math/Transformation.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Explorer {
class BaseLayer : public Layer {

public: // Setting up layer
  BaseLayer(MTL::Device* device, AppProperties* config);
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
  virtual MTL::RenderPipelineState* getRenderPipelineState(std::string shaderName, bool serialize);
  virtual MTL::DepthStencilState* getDepthStencilState();
  virtual MTL::SamplerState* getSamplerState();

private: // Rendering
  virtual void drawLight(MTL::RenderCommandEncoder* encoder, LightSource* light);
  virtual void drawMesh(MTL::RenderCommandEncoder* encoder, Mesh* mesh);

private:
  MTL::Device* device;
  MTL::CommandQueue* commandQueue;

private: // States
  MTL::RenderPipelineState* generalPipelineState;
  MTL::RenderPipelineState* lightPipelineState;
  MTL::DepthStencilState* depthStencilState;
  MTL::SamplerState* samplerState;

private:
  Camera camera;
  LightSource* light;
  Mesh* quad;
  Mesh* pyramid;
  Mesh* cube;
  MTL::Texture* island;
  std::vector<Mesh*> f16;
  std::vector<Mesh*> cruiser;
  float t;
};
}; // namespace Explorer

#pragma once
#include <Control/AppProperties.h>
#include <DB/Repository.hpp>
#include <Layer/Layer.h>
#include <Math/Transformation.h>
#include <Model/Camera.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace EXP {
class BaseLayer : public Layer {

public: // Setting up layer
  BaseLayer(MTL::Device* device, AppProperties* config);
  ~BaseLayer();

public: // Event
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* buffer) override;
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

private:
  //MTL::CommandQueue* commandQueue;

	float mouseX = 0;
	float mouseY = 0;

private: // States
  MTL::RenderPipelineState* generalPipelineState;
  MTL::RenderPipelineState* lightPipelineState;
  MTL::DepthStencilState* depthStencilState;
  MTL::SamplerState* samplerState;
	MTL::VertexDescriptor* _vertexDescriptor;
	OrthographicCamera camera;
  Light* light;
  Model* quad;
  Model* pyramid;
  Model* cube;
  MTL::Texture* island;
  Model* f16;
  Model* cruiser;
	Model* bugatti;
	Model* sphere;
  float t;
};
}; // namespace EXP

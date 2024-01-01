#pragma once
#include <Layer/Layer.h>
#include <Model/MeshFactory.h>
#include <pch.h>

namespace Explorer {
class BaseLayer : public Layer {

public:
  BaseLayer(MTL::Device *device);
  ~BaseLayer();

  virtual void buildPipeline();
  virtual void buildMeshes();
  virtual NS::String *open(std::string path);
  virtual MTL::Library *getLibrary(std::string path);
  virtual MTL::RenderPipelineDescriptor *getRenderPipelineDescriptor(
      std::string path, std::string vertexFnName, std::string fragmentFnName);
  virtual MTL::VertexDescriptor *getVertexDescriptor();
  virtual MTL::RenderPipelineState *getRenderPipelineState(std::string shaderName, bool serialize);
  virtual MTL::BinaryArchive *createBinaryArchive();
  virtual bool writeBinaryArchive(MTL::RenderPipelineDescriptor *, std::string path);
  virtual bool readBinaryArchive(std::string path);
  virtual NS::String *nsString(std::string str);
  virtual NS::URL *nsUrl(std::string path);
  virtual void onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) override;
public:
	virtual void onEvent(Event& event) override;
	virtual bool onKeyPressed(KeyPressedEvent &event);
  virtual bool onKeyReleased(KeyReleasedEvent &event);
  virtual bool onMouseButtonPressed(MouseButtonPressedEvent &event);
  virtual bool onMouseButtonReleased(MouseButtonReleasedEvent &event);
  virtual bool onMouseMove(MouseMoveEvent &event);

private:
  virtual void printError(NS::Error *error);
  MTL::Device *device;
  MTL::CommandQueue *commandQueue;
  MTL::RenderPipelineState *generalPipeline;
  MTL::Buffer *mesh;
  NS::StringEncoding stringEncoding;
  MeshFactory::Mesh quadMesh;
  float t;
};
}; // namespace Explorer

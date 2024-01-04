#include "Metal/MTLVertexDescriptor.hpp"
#include <Layer/BaseLayer.h>
#include <Math/Transformation.h>
#include <Renderer/Buffer.h>

Explorer::BaseLayer::BaseLayer(MTL::Device* device)
    : Layer(device->retain()),
      repository(ShaderRepository(device->retain(), "/Users/ramonsmits/Code/Explorer/src/Shaders/")
      ) {
  this->device = device;
  this->buildPipeline();
  this->buildMeshes();
}

Explorer::BaseLayer::~BaseLayer() {
  this->quadMesh->vertexBuffer->release();
  this->quadMesh->indexBuffer->release();
  this->commandQueue->release();
  this->device->release();
  this->generalPipeline->release();
  this->mesh->release();
  this->depthStencilState->release();
}

void Explorer::BaseLayer::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(BaseLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(BaseLayer::onKeyReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(BaseLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(BaseLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(BaseLayer::onMouseMove));
}

bool Explorer::BaseLayer::onKeyPressed(KeyPressedEvent& event) { return true; }

bool Explorer::BaseLayer::onKeyReleased(KeyReleasedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseButtonPressed(MouseButtonPressedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseButtonReleased(MouseButtonReleasedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseMove(MouseMoveEvent& event) { return true; }

void Explorer::BaseLayer::buildPipeline() {
  this->generalPipeline = this->getRenderPipelineState("General", true);
  this->depthStencilState = this->getDepthStencilState();
  this->projection = Transformation::perspective(45.0f, 1.0f, 0.1f, 100.0f);
}

void Explorer::BaseLayer::buildMeshes() {
  // Computation of matrices are down right -> left.
  // Meaning you first need to translate, then rotate, then scale
  this->pyramid = MeshFactory::pyramid(this->device)->scale(0.25f)->translate({-0.5f, 0.0f, -2.0f});
  this->mesh = MeshFactory::triangle(this->device);
  this->quadMesh = MeshFactory::quad(this->device)->scale(0.9f)->translate({-0.5f, 0.0f, -3.0f});
  this->cube = MeshFactory::cube(this->device)->scale(0.5f)->translate({0.25, 0.25, -4});
  this->light = MeshFactory::light(this->device);
}

MTL::RenderPipelineDescriptor*
Explorer::BaseLayer::getRenderPipelineDescriptor(std::string shaderName) {
  MTL::Library* library = this->repository.getLibrary(shaderName);
  MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  // Add flags to state adding binary functions is supported
  descriptor->setSupportAddingVertexBinaryFunctions(true);
  descriptor->setSupportAddingFragmentBinaryFunctions(true);

  // Add functions to the pipeline descriptor
  descriptor->setVertexFunction(library->newFunction(nsString("vertexMain" + shaderName)));
  descriptor->setFragmentFunction(library->newFunction(nsString("fragmentMain" + shaderName)));
  descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB
  );

  // Declare layout to be used in communication between CPU and GPU
  BufferLayout layout = {
      {ShaderDataType::Float4, "position"},
      {ShaderDataType::Float3,    "color"}
  };
  descriptor->setVertexDescriptor(this->getVertexDescriptor(&layout));
  library->release();
  return descriptor;
}

MTL::DepthStencilState* Explorer::BaseLayer::getDepthStencilState() {
  MTL::DepthStencilDescriptor* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
  descriptor->setDepthWriteEnabled(true);
  descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
  return device->newDepthStencilState(descriptor);
}

MTL::VertexDescriptor* Explorer::BaseLayer::getVertexDescriptor(BufferLayout* layout) {
  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  int i = 0;
  int stride = 0;
  for (BufferElement element : layout->getElements()) {
    MTL::VertexAttributeDescriptor* positionDescriptor = vertexDescriptor->attributes()->object(i);
    positionDescriptor->setFormat((MTL::VertexFormat)element.type);
    positionDescriptor->setOffset(stride);
    positionDescriptor->setBufferIndex(0);
    stride += element.size;
    i += 1;
  }

  // Layout descriptor for both pos & color as one layout
  MTL::VertexBufferLayoutDescriptor* layoutDescriptor = vertexDescriptor->layouts()->object(0);
  layoutDescriptor->setStride(stride);

  return vertexDescriptor;
}

MTL::RenderPipelineState*
Explorer::BaseLayer::getRenderPipelineState(std::string shaderName, bool serialize) {
  // Compiling the shader into an archive (wrapped by pipeline descriptor)
  MTL::RenderPipelineDescriptor* descriptor = this->getRenderPipelineDescriptor(shaderName);

  // Setting & getting descriptor directly from binary
  if (serialize) repository.write(descriptor, shaderName);

  WARN("Shader deserialization not working. Compiling shaders ...");
  // MTL::RenderPipelineDescriptor* archive = this->readBinaryArchive(mlibPath);

  // Generating pipeline state from pipeline descriptor
  DEBUG("Attempting to generate state from descriptor ...");
  NS::Error* newPipelineStateError = nullptr;
  MTL::RenderPipelineState* state =
      device->newRenderPipelineState(descriptor, &newPipelineStateError);
  if (!state) printError(newPipelineStateError);
  newPipelineStateError->release();
  descriptor->release();
  DEBUG("Returning state ...");
  return state;
}

void Explorer::BaseLayer::drawLight(MTL::RenderCommandEncoder* encoder, LightSource* light) {

  simd::float4 lighting = {700.0f, 700.0f, 0.0f, 1.0f};
  encoder->setFragmentBytes(
      &lighting,            // Setting a buffer
      sizeof(simd::float4), // Size of the data
      1                     // Index of the buffer
  );
}

void Explorer::BaseLayer::drawMesh(MTL::RenderCommandEncoder* encoder, Mesh* mesh) {
  encoder->setVertexBuffer(
      mesh->vertexBuffer, // The data to use for vertex buffer
      0,                  // The offset of the vertex buffer
      0                   // The index in the buffer to start drawing from
  );

  simd::float4x4 transform = projection * mesh->transform();
  encoder->setVertexBytes(
      &transform,             // The data set in GPU
      sizeof(simd::float4x4), // The size of data set in GPU
      1                       // The location of data: [[buffer(1)]]
  );
  encoder->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle, // Type of object to draw
      mesh->indexBuffer->length(),               // Number of elements in the index buffer
      MTL::IndexType::IndexTypeUInt16,           // The data type of the data in buffer
      mesh->indexBuffer,                         // The index buffer holding the indice data
      NS::UInteger(0),                           // The index buffer offset
      NS::UInteger(1)                            // For instanced rendering. We render 1 object
  );
}

void Explorer::BaseLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) {
  t += 1.0f;
  if (t > 360) t -= 360.0f;
	
	this->drawLight(encoder, this->light);
  encoder->setRenderPipelineState(this->generalPipeline);
  encoder->setDepthStencilState(this->depthStencilState);

  this->pyramid->rotate(Transformation::yRotation(t) * Transformation::xRotation(80));

  this->cube->rotate(Transformation::xRotation(t) * Transformation::zRotation(t));
  this->drawMesh(encoder, this->quadMesh);
  this->drawMesh(encoder, this->pyramid);
  this->drawMesh(encoder, this->cube);
}

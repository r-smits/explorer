#include <DB/Repository.h>
#include <Events/IOState.h>
#include <Events/KeyCodes.h>
#include <Layer/BaseLayer.h>
#include <Math/Transformation.h>
#include <Renderer/Buffer.h>
#include <Renderer/Descriptor.h>
#include <Renderer/Layout.h>

Explorer::BaseLayer::BaseLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config) {
  this->device = device;
  this->buildPipeline();
  this->buildMeshes();
}

Explorer::BaseLayer::~BaseLayer() {
  this->commandQueue->release();
  this->device->release();
  this->generalPipelineState->release();
  this->depthStencilState->release();
  this->samplerState->release();
}

void Explorer::BaseLayer::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(BaseLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(BaseLayer::onKeyReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(BaseLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(BaseLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(BaseLayer::onMouseMove));
}

bool Explorer::BaseLayer::onKeyPressed(KeyPressedEvent& event) { return false; }

bool Explorer::BaseLayer::onKeyReleased(KeyReleasedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseButtonPressed(MouseButtonPressedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseButtonReleased(MouseButtonReleasedEvent& event) { return true; }

bool Explorer::BaseLayer::onMouseMove(MouseMoveEvent& event) { return true; }

void Explorer::BaseLayer::buildPipeline() {
  this->generalPipelineState = this->getRenderPipelineState("General", false);
  // this->lightPipelineState = this->getRenderPipelineState("Light", false);
  this->depthStencilState = this->getDepthStencilState();
  this->samplerState = this->getSamplerState();
  this->camera = Camera();
}

void Explorer::BaseLayer::buildMeshes() {

  pyramid = MeshFactory::pyramid(this->device, config->texturePath + "island.jpg");
  pyramid->scale = 0.25f;
  pyramid->position = {-0.5f, 0.0f, -2.0f};

  quad = MeshFactory::quad(device, config->texturePath + "island.jpg");
  quad->scale = 0.9f;
  quad->position = {0.0f, 0.0f, -3.0f};

  cube = MeshFactory::cube(this->device, config->texturePath + "island.jpg");
  cube->scale = 0.25f;
  cube->position = {0.25, 0.25, -2};

  light = MeshFactory::light(this->device);

  f16 = Repository::Meshes::read(device, config->meshPath + "f16/f16");

	cruiser = Repository::Meshes::read(device, config->meshPath + "cruiser/cruiser");

	for (Mesh* mesh : f16) {
	mesh->position.z -= 2.5;
	}

  camera.position.z;
  // this->light->position = {-0.25, 0.3, 0};
}

MTL::RenderPipelineDescriptor*
Explorer::BaseLayer::getRenderPipelineDescriptor(std::string shaderName) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, config->shaderPath + shaderName);
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

  descriptor->setVertexDescriptor(Renderer::Descriptor::vertex(device, &Renderer::Layouts::vertex));
  library->release();
  return descriptor;
}

MTL::DepthStencilState* Explorer::BaseLayer::getDepthStencilState() {
  MTL::DepthStencilDescriptor* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
  descriptor->setDepthWriteEnabled(true);
  descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
  return device->newDepthStencilState(descriptor);
}

MTL::SamplerState* Explorer::BaseLayer::getSamplerState() {
  MTL::SamplerDescriptor* descriptor = MTL::SamplerDescriptor::alloc()->init();
  descriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
  descriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
  return device->newSamplerState(descriptor);
}

MTL::RenderPipelineState*
Explorer::BaseLayer::getRenderPipelineState(std::string shaderName, bool serialize) {
  // Compiling the shader into an archive (wrapped by pipeline descriptor)
  MTL::RenderPipelineDescriptor* descriptor = this->getRenderPipelineDescriptor(shaderName);

  // Setting & getting descriptor directly from binary
  if (serialize) Repository::Shaders::write(device, descriptor, config->shaderPath + shaderName);

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

  CGRect bounds = ViewAdapter::bounds();
  simd::float4 lighting = {1000.0f, 850.0f, 00.0f, 1.0f};
  simd::float4 transformation = camera.f4x4() * light->f4x4() * lighting;

  // simd::float4 transform = light-> * light->f4x4();
  encoder->setFragmentBytes(
      &lighting,            // Setting a buffer
      sizeof(simd::float4), // Size of the data
      1                     // Index of the buffer
  );
}

void Explorer::BaseLayer::drawMesh(MTL::RenderCommandEncoder* encoder, Mesh* mesh) {

  encoder->setFragmentTexture(mesh->texture,
                              0); // Setting texture to render onto mesh

  encoder->setVertexBuffer(
      mesh->vertexBuffer, // The data to use for vertex buffer
      0,                  // The offset of the vertex buffer
      0                   // The index in the buffer to start drawing from
  );

  simd::float4x4 transform = camera.f4x4() * mesh->f4x4();
  encoder->setVertexBytes(
      &transform,             // The data set in GP
      sizeof(simd::float4x4), // The size of data set in GPU
      1                       // The location of data: [[buffer(1)]]
  );

  for (Submesh* submesh : mesh->submeshes) {
    encoder->drawIndexedPrimitives(
        submesh->primitiveType, // Type of object to draw
        submesh->indexCount,    // Number of elements in the index buffer
        submesh->indexType,     // The data type of the data in buffer
        submesh->indexBuffer,   // The index buffer holding the indice data
        submesh->offset,        // The index buffer offset
        NS::UInteger(1)         // For instanced rendering. We render 1 object
    );
  }
}

void Explorer::BaseLayer::checkIO() {
  if (Explorer::IO::isPressed(KEY_D)) {
    camera.rotation = Transformation::translation({0.0f, 0.0f, -2.5f}) *
                      Transformation::yRotation(-camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (Explorer::IO::isPressed(KEY_A)) {
    camera.rotation = Transformation::translation({0.0f, 0.0f, -2.5f}) *
                      Transformation::yRotation(camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (IO::isPressed(KEY_W)) {
    camera.rotation = Transformation::translation({0.0f, 0.0f, -2.5f}) *
                      Transformation::xRotation(-camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (IO::isPressed(KEY_S)) {
    camera.rotation = Transformation::translation({0.0f, 0.0f, -2.5f}) *
                      Transformation::xRotation(camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }
}

void Explorer::BaseLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) {
  t += 1.0f;
  if (t > 360) t -= 360.0f;

  checkIO();

  encoder->setRenderPipelineState(this->generalPipelineState);
  encoder->setDepthStencilState(this->depthStencilState);
  encoder->setFragmentSamplerState(this->samplerState, 0);

  //pyramid->rotation = Transformation::yRotation(t) * Transformation::xRotation(80);
  //cube->rotation = Transformation::xRotation(t) * Transformation::zRotation(t);

  // this->drawLight(encoder, light);
  //this->drawMesh(encoder, quad);

  // this->drawMesh(encoder, pyramid);
  // this->drawMesh(encoder, cube);

  for (Mesh* mesh : f16) {
    drawMesh(encoder, mesh);
  }
}

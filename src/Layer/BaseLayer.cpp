#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Events/KeyCodes.h>
#include <Layer/BaseLayer.h>
#include <Math/Transformation.h>
#include <Renderer/Buffer.h>
#include <Renderer/Descriptor.h>
#include <Renderer/Draw.h>
#include <Renderer/Layout.h>
#include <Renderer/State.h>

Explorer::BaseLayer::BaseLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config) {
  this->device = device;
  this->buildPipeline();
  this->buildMeshes();
	DEBUG("BaseLayer :: Initialization done.");
}

Explorer::BaseLayer::~BaseLayer() {
  this->commandQueue->release();
  this->device->release();
  this->generalPipelineState->release();
  this->depthStencilState->release();
  this->samplerState->release();

  delete f16;
  delete bugatti;
  delete cruiser;
  delete pyramid;
  delete quad;
  delete cube;
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

bool Explorer::BaseLayer::onMouseMove(MouseMoveEvent& event) {
	mouseX = event.getX();
	mouseY = event.getY();
	return true; 
}

void Explorer::BaseLayer::buildPipeline() {
  this->generalPipelineState = this->getRenderPipelineState("General", false);

  this->depthStencilState = Renderer::State::depthStencil(device); // Z coordinate interpretation
  this->samplerState = Renderer::State::sampler(device);           // For textures
  this->camera = Camera(); // Transformations specific to the view
}

void Explorer::BaseLayer::buildMeshes() {

  // pyramid = MeshFactory::pyramid(this->device, config->texturePath +
  // "island.jpg"); pyramid->scale = 0.25f; pyramid->position = {-0.5f, 0.0f,
  // -2.0f};

  //quad = MeshFactory::quad(device, config->texturePath + "island.jpg");
  //quad->scale(0.25f);
  //quad->translate({0.0f, 0.75f, -2.5f});
	//quad->f4x4();
	
	// cube = MeshFactory::cube(this->device, config->texturePath + "island.jpg");
  // cube->scale = 0.25f;
  // cube->position = {0.25, 0.25, -2};

	// cruiser = Repository::Meshes::read(device, config->meshPath +
  // "cruiser/cruiser");

  // bugatti = Repository::Meshes::read(device, config->meshPath +
  // "bugatti/bugatti"); bugatti->position.z -= 80;

  sphere = Repository::Meshes::read(device, config->meshPath + "sphere/sphere", false, false); 
	sphere->translate({0.0f, 0.30f, -2.5f})->scale(0.1)->f4x4();
	
  f16 = Repository::Meshes::read(device, config->meshPath + "f16/f16");
  f16->translate({0.0f, 0.0f, -2.5f});

	cruiser = Repository::Meshes::read(device, config->meshPath + "cruiser/cruiser");
	cruiser->translate({0.0f, 0.0f, -2.5f})->scale(0.5f)->f4x4();

	light = MeshFactory::light(this->device);
	light->translate({0.0f, 0.5f, -2.5f});

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

void Explorer::BaseLayer::checkIO() {
	
	//light->translate({mouseX, mouseY, 0.0f});
	//light->f4x4();

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
                      Transformation::xRotation(camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (IO::isPressed(KEY_S)) {
    camera.rotation = Transformation::translation({0.0f, 0.0f, -2.5f}) *
                      Transformation::xRotation(-camera.rotateSpeed) *
                      Transformation::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }
	camera.f4x4();

  if (IO::isPressed(ARROW_UP)) {
    f16->rotate(Transformation::xRotation(camera.rotateSpeed) * f16->rotation);
  }
  if (IO::isPressed(ARROW_DOWN)) {
    f16->rotate(Transformation::xRotation(-camera.rotateSpeed) * f16->rotation);
  }
  if (IO::isPressed(ARROW_LEFT)) {
    f16->rotate(Transformation::yRotation(camera.rotateSpeed) * f16->rotation);
  }
  if (IO::isPressed(ARROW_RIGHT)) {
    f16->rotate(Transformation::yRotation(-camera.rotateSpeed) * f16->rotation);
  }
	f16->f4x4();
}

void Explorer::BaseLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) {
  t += 1.0f;
  if (t > 360) t -= 360.0f;

  checkIO();
  encoder->setRenderPipelineState(this->generalPipelineState);
  encoder->setDepthStencilState(this->depthStencilState);
  encoder->setFragmentSamplerState(this->samplerState, 0);
	
	(t < 90 || t > 270) ? sphere->translate({1.0/180, 0.0f, 0.0f})->f4x4() : 
		sphere->translate({-1.0/180, 0.0f, -0.0f})->f4x4();

	light->data.position = sphere->position;
	//(t < 90 || t > 170) ? light->translate({1.0/180, 0.0f, 0.0f}) : light->translate({-1.0/180, 0.0f, 0.0f});

  Renderer::Draw::light(encoder, camera, light);
  Renderer::Draw::model(encoder, camera, sphere);
  Renderer::Draw::model(encoder, camera, f16);
}

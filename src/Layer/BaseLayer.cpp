#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Events/KeyCodes.h>
#include <Layer/BaseLayer.h>
#include <Math/Transformation.h>
#include <Renderer/Renderer.h>

Explorer::BaseLayer::BaseLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config) {

  _vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexUnwoven);
  this->buildPipeline();
  this->buildMeshes();
  DEBUG("BaseLayer :: Initialization done.");
}

Explorer::BaseLayer::~BaseLayer() {
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
  auto descriptor =
      Renderer::Descriptor::render(device, _vertexDescriptor, config->shaderPath + "General");
  if (false) Repository::Shaders::write(device, descriptor, config->shaderPath + "General");
  generalPipelineState = Renderer::State::render(device, descriptor);
  depthStencilState = Renderer::State::depthStencil(device); // Z coordinate interpretation

  camera = OrthographicCamera(); // Transformations specific to the view
  camera.rotate(
      Transformation::translation({0.0f, 0.0f, -2.5f}) * Transformation::xRotation(-30.0f) *
      Transformation::yRotation(-45.0f) * Transformation::translation({0.0f, 0.0f, 2.5f})
  );
  camera.project();
}

void Explorer::BaseLayer::buildMeshes() {

  // pyramid = MeshFactory::pyramid(this->device, config->texturePath +
  // "island.jpg"); pyramid->scale = 0.25f; pyramid->position = {-0.5f, 0.0f,
  // -2.0f};

  // quad = MeshFactory::quad(device, config->texturePath + "island.jpg");
  // quad->scale(0.25f);
  // quad->translate({0.0f, 0.75f, -2.5f});
  // quad->f4x4();

  // cube = MeshFactory::cube(this->device, config->texturePath + "island.jpg");
  // cube->scale = 0.25f;
  // cube->position = {0.25, 0.25, -2};

  // cruiser = Repository::Meshes::read(device, config->meshPath +
  // "cruiser/cruiser");

  // bugatti = Repository::Meshes::read(device, config->meshPath +
  // "bugatti/bugatti"); bugatti->position.z -= 80;

  sphere = Repository::Meshes::read(device, _vertexDescriptor, config->meshPath + "sphere/sphere", false, false);
  sphere->translate({0.0f, 0.30f, -2.5f})->scale(0.1)->f4x4();

  f16 = Repository::Meshes::read2(device, _vertexDescriptor, config->meshPath + "f16/f16");
  f16->translate({0.0f, 0.0f, -2.5f});

  cruiser = Repository::Meshes::read(device, _vertexDescriptor, config->meshPath + "cruiser/cruiser");
  cruiser->translate({0.0f, 0.0f, -2.5f})->scale(0.5f)->f4x4();

  light = MeshFactory::light(this->device);
  light->translate({0.0f, 0.5f, -2.5f});
}

void Explorer::BaseLayer::checkIO() {

  // light->translate({mouseX, mouseY, 0.0f});
  // light->f4x4();

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
  encoder->setRenderPipelineState(generalPipelineState);
  encoder->setDepthStencilState(depthStencilState);

  (t < 90 || t > 270) ? sphere->translate({1.0 / 180, 0.0f, 0.0f})->f4x4()
                      : sphere->translate({-1.0 / 180, 0.0f, -0.0f})->f4x4();

  light->data.position = sphere->position;

  Renderer::Draw::light(encoder, camera, light);
  Renderer::Draw::model(encoder, camera, sphere);
  Renderer::Draw::model(encoder, camera, f16);
}

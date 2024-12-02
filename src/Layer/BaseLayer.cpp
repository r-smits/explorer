#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Events/KeyCodes.h>
#include <Layer/BaseLayer.h>
#include <Math/Transformation.h>
#include <Renderer/Renderer.h>

EXP::BaseLayer::BaseLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config) {

  _vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);
  this->buildPipeline();
  this->buildMeshes();
  DEBUG("BaseLayer :: Initialization done.");
}

EXP::BaseLayer::~BaseLayer() {
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

void EXP::BaseLayer::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(BaseLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(BaseLayer::onKeyReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(BaseLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(BaseLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(BaseLayer::onMouseMove));
}

bool EXP::BaseLayer::onKeyPressed(KeyPressedEvent& event) { return false; }

bool EXP::BaseLayer::onKeyReleased(KeyReleasedEvent& event) { return true; }

bool EXP::BaseLayer::onMouseButtonPressed(MouseButtonPressedEvent& event) { return true; }

bool EXP::BaseLayer::onMouseButtonReleased(MouseButtonReleasedEvent& event) { return true; }

bool EXP::BaseLayer::onMouseMove(MouseMoveEvent& event) {
  mouseX = event.getX();
  mouseY = event.getY();
  return true;
}

void EXP::BaseLayer::buildPipeline() {
  auto descriptor =
      Renderer::Descriptor::render(device, _vertexDescriptor, config->shaderPath + "General");
  if (false) Repository::Shaders::write(device, descriptor, config->shaderPath + "General");
  generalPipelineState = Renderer::State::render(device, descriptor);
  depthStencilState = Renderer::State::depthStencil(device); // Z coordinate interpretation

  camera = OrthographicCamera(); // Transformations specific to the view
  camera.rotate(
      EXP::MATH::translation({0.0f, 0.0f, -2.5f}) * EXP::MATH::xRotation(-30.0f) *
      EXP::MATH::yRotation(-45.0f) * EXP::MATH::translation({0.0f, 0.0f, 2.5f})
  );
  camera.project();
}

void EXP::BaseLayer::buildMeshes() {

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
  sphere->move({0.0f, 0.30f, -2.5f})->scale(0.1)->f4x4();

  f16 = Repository::Meshes::read2(device, _vertexDescriptor, config->meshPath + "f16/f16");
  f16->move({0.0f, 0.0f, -2.5f});

  cruiser = Repository::Meshes::read(device, _vertexDescriptor, config->meshPath + "cruiser/cruiser");
  cruiser->move({0.0f, 0.0f, -2.5f})->scale(0.5f)->f4x4();

  light = EXP::MeshFactory::light(this->device);
  light->translate({0.0f, 0.5f, -2.5f});
}

void EXP::BaseLayer::checkIO() {

  // light->translate({mouseX, mouseY, 0.0f});
  // light->f4x4();

  if (EXP::IO::isPressed(KEY_D)) {
    camera.rotation = EXP::MATH::translation({0.0f, 0.0f, -2.5f}) *
                      EXP::MATH::yRotation(-camera.rotateSpeed) *
                      EXP::MATH::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (EXP::IO::isPressed(KEY_A)) {
    camera.rotation = EXP::MATH::translation({0.0f, 0.0f, -2.5f}) *
                      EXP::MATH::yRotation(camera.rotateSpeed) *
                      EXP::MATH::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (IO::isPressed(KEY_W)) {
    camera.rotation = EXP::MATH::translation({0.0f, 0.0f, -2.5f}) *
                      EXP::MATH::xRotation(camera.rotateSpeed) *
                      EXP::MATH::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }

  if (IO::isPressed(KEY_S)) {
    camera.rotation = EXP::MATH::translation({0.0f, 0.0f, -2.5f}) *
                      EXP::MATH::xRotation(-camera.rotateSpeed) *
                      EXP::MATH::translation({0.0f, 0.0f, 2.5f}) * camera.rotation;
  }
  camera.f4x4();

  if (IO::isPressed(ARROW_UP)) {
    f16->rotate(EXP::MATH::xRotation(camera.rotateSpeed));
  }
  if (IO::isPressed(ARROW_DOWN)) {
    f16->rotate(EXP::MATH::xRotation(-camera.rotateSpeed));
  }
  if (IO::isPressed(ARROW_LEFT)) {
    f16->rotate(EXP::MATH::yRotation(camera.rotateSpeed));
  }
  if (IO::isPressed(ARROW_RIGHT)) {
    f16->rotate(EXP::MATH::yRotation(-camera.rotateSpeed));
  }
  f16->f4x4();
}

void EXP::BaseLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* encoder) {
  t += 1.0f;
  if (t > 360) t -= 360.0f;

  checkIO();
  encoder->setRenderPipelineState(generalPipelineState);
  encoder->setDepthStencilState(depthStencilState);

  (t < 90 || t > 270) ? sphere->move({1.0 / 180, 0.0f, 0.0f})->f4x4()
                      : sphere->move({-1.0 / 180, 0.0f, -0.0f})->f4x4();

  light->data.position = sphere->meshes[0]->position;

  Renderer::Draw::light(encoder, camera, light);
  Renderer::Draw::model(encoder, camera, sphere);
  Renderer::Draw::model(encoder, camera, f16);
}

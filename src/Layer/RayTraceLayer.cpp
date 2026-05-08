#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/ImGuiAPI.h>
#include <Layer/RayTraceLayer.h>
#include <Math/Transformation.h>
#include <Metal/MTLCommandBuffer.hpp>
#include <Metal/MTLCommandEncoder.hpp>
#include <Metal/MTLComputePass.hpp>
#include <Renderer/Renderer.h>
#include <Renderer/Types.h>

using s_repo = Repository::Shaders;
using r_acc = Renderer::Acceleration;

EXP::RayTraceLayer::RayTraceLayer(MTK::View *view,
                                  std::shared_ptr<const AppProperties> _config)
    : Layer(view, _config), queue(view->device()->newCommandQueue()) {
  auto device = view->device();
  MTL::Library *gbufferLib =
      s_repo::readLibrary(device, config->shader_path / "GBuffer");
  MTL::Library *temporalReuseLib =
      s_repo::readLibrary(device, config->shader_path / "RESTIR");
  MTL::Function *gbufferFn = gbufferLib->newFunction(EXP::nsString("g_buffer"));
  MTL::Function *temporalReuseFn =
      temporalReuseLib->newFunction(EXP::nsString("temporal_reuse"));

  _gbufferState = Renderer::State::Compute(device, gbufferFn);
  _temporalReuseState = Renderer::State::Compute(device, temporalReuseFn);

  _vertexDescriptor =
      Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);
  CGRect frame = ViewAdapter::bounds();
  _gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  _resolution = {(float)_gridSize.width, (float)_gridSize.height,
                 (float)_gridSize.depth};
	_threadGroupSize = calcGridsize(_temporalReuseState);
	_temporalDescriptor = MTL::ComputePassDescriptor::alloc()->init();
	_temporalDescriptor->retain();
	
	MTL::TextureDescriptor* desc = MTL::TextureDescriptor::texture2DDescriptor(
			MTL::PixelFormat::PixelFormatRGBA8Unorm_sRGB, 
			_gridSize.width * 2,
      _gridSize.height * 2,
      false
	);
	desc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
	desc->setStorageMode(MTL::StorageModePrivate);
	_outputTexture = device->newTexture(desc)->retain();

	buildModels(device);
	buildAccelerationStructures(device);
	initialize_imgui(view);
}

void EXP::RayTraceLayer::buildModels(MTL::Device *device) {

  EXP::SCENE::addTexture(device, "reservoirs",
                         Renderer::TextureAccess::READ_WRITE);
  EXP::SCENE::addTexture(device, "accumulation",
                         Renderer::TextureAccess::READ_WRITE);

  EXP::SCENE::addModel(device, _vertexDescriptor, config->mesh_path / "f16/f16",
                       "f16");
  EXP::SCENE::addModel(device, _vertexDescriptor,
                       config->mesh_path / "sphere/sphere", "sphere1");
  EXP::SCENE::addModel(device, _vertexDescriptor,
                       config->mesh_path / "sphere/sphere", "sphere2");
	//EXP::SCENE::addModel(device, _vertexDescriptor,
  //                     config->mesh_path / "sphere/sphere", "sphere3");


  EXP::Model *f16 = EXP::SCENE::getModel("f16");
  EXP::Model *sphere1 = EXP::SCENE::getModel("sphere1");
  EXP::Model *sphere2 = EXP::SCENE::getModel("sphere2");
	// EXP::Model *sphere3 = EXP::SCENE::getModel("sphere3");

  f16\
		->move({0.0f, 0.0f, 0.0f})
		->scale(.5f);

  sphere1\
		->setEmissive(true)
    ->setColor({4.0f, 4.0f, 1.f, .0f})
    ->scale(.1f)
    ->move({-.3f, .6f, .1f});

  sphere2\
		->setColor({0.0f, 1.0f, 0.0f, 1.0f})
    ->scale(.125f)
    ->move({-.2f, .3f, -.3f});
	
	/**
	sphere3\
		->setEmissive(true)
    ->setColor({.0f, .0f, 1.f, .0f})
    ->scale(.05f)
    ->move({.5f, .4f, .4f});
	**/

  EXP::SCENE::buildBindlessScene(device);
  EXP::SCENE::getCamera()->setIsometric();
}

MTL::Size
EXP::RayTraceLayer::calcGridsize(const MTL::ComputePipelineState *state) {
  auto threadGroupWidth = state->threadExecutionWidth();
  auto threadGroupHeight =
      state->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  DEBUG("Thread group width x height: " + std::to_string(threadGroupWidth) +
        " x " + std::to_string(threadGroupHeight));
  return MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);
}

void EXP::RayTraceLayer::buildAccelerationStructures(MTL::Device *device) {
  // events to wait for while building
  _dispatchEvent = device->newEvent();
  _buildEvent = device->newEvent();

  // primitive acc structures
  int vStride = _vertexDescriptor->layouts()->object(0)->stride();
  int pStride = _vertexDescriptor->layouts()->object(1)->stride();
  _primitiveDescriptors = Renderer::Descriptor::primitives(
      EXP::SCENE::getMeshes(), vStride, pStride);
  MTL::AccelerationStructureSizes primitiveSizes =
      Renderer::Acceleration::sizes(device, _primitiveDescriptors);
  _heap = Renderer::Heap::primitives(device, primitiveSizes);
  _primitiveAccStructures = Renderer::Acceleration::primitives(
      device, _heap, queue, _primitiveDescriptors, primitiveSizes, _buildEvent);

  // instance acc structure
  _instanceDescriptor =
      Renderer::Descriptor::instance(device, _primitiveAccStructures,
                                     EXP::SCENE::getMeshes())
          ->retain();
  _instanceSizes = device->accelerationStructureSizes(_instanceDescriptor);
  _scratchBuffer = device
                       ->newBuffer(_instanceSizes.buildScratchBufferSize,
                                   MTL::ResourceStorageModePrivate)
                       ->retain();
  _instanceAccStructure = device->newAccelerationStructure(
      _instanceSizes.accelerationStructureSize);
  _instanceAccStructure = Renderer::Acceleration::instance(
      device, queue, _instanceAccStructure, _instanceDescriptor, _scratchBuffer,
      _buildEvent);
}

void EXP::RayTraceLayer::rebuildAccelerationStructures(MTK::View *view) {
  _instanceDescriptor = Renderer::Descriptor::updateTransformationMatrix(
      EXP::SCENE::getMeshes(), _instanceDescriptor);
  _instanceAccStructure = Renderer::Acceleration::instance(
      device, queue, _instanceAccStructure, _instanceDescriptor, _scratchBuffer,
      _buildEvent);
}

void EXP::RayTraceLayer::onUpdate(MTK::View *view,
                                  MTL::RenderCommandEncoder *notUsed) {

  // Scene action & update acceleration structure
	//
  // Update camera part of the bindless scene
  EXP::SCENE::updateBindlessScene(view->device());

  rebuildAccelerationStructures(view);

  // ------------------------------ //
  // Temporal Re-use RESTIR GI	  //
  // ------------------------------ //
  MTL::CommandBuffer *command_buffer = queue->commandBuffer();
  command_buffer->encodeWait(_buildEvent, 2);
  MTL::ComputeCommandEncoder *compute_encoder =
      command_buffer->computeCommandEncoder(_temporalDescriptor);

  compute_encoder->setComputePipelineState(_temporalReuseState);
  compute_encoder->setTexture(_outputTexture, 0);

  compute_encoder->useHeap(_heap);
  compute_encoder->setAccelerationStructure(_instanceAccStructure, 1);
  compute_encoder->useResource(_instanceAccStructure, MTL::ResourceUsageRead);

  const std::vector<MTL::Resource *> &resources = EXP::SCENE::getResources();
  compute_encoder->useResources(resources.data(), resources.size(),
                                MTL::ResourceUsageRead |
                                    MTL::ResourceUsageSample);
  compute_encoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);
  compute_encoder->dispatchThreads(_gridSize, _threadGroupSize);
  compute_encoder->endEncoding();

  //
  // Blit RenderPass
  //
  MTL::RenderPassDescriptor *render_pass_descriptor =
      view->currentRenderPassDescriptor();
  MTL::RenderPassColorAttachmentDescriptor *color_attachment =
      render_pass_descriptor->colorAttachments()->object(0);
  color_attachment->setLoadAction(MTL::LoadActionClear);
  color_attachment->setStoreAction(MTL::StoreActionStore);

  MTL::RenderCommandEncoder *render_encoder =
      command_buffer->renderCommandEncoder(render_pass_descriptor);
  imgui_on_update(view, command_buffer, render_pass_descriptor, render_encoder,
                  _outputTexture);
  render_encoder->endEncoding();
  command_buffer->presentDrawable(view->currentDrawable());
  command_buffer->commit();
}

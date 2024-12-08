#include "Math/Transformation.h"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLComputePass.hpp"
#include "Metal/MTLTexture.hpp"
#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>


EXP::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {
	
	MTL::Library* gBufferLib = Repository::Shaders::readLibrary(device, config->shaderPath + "GBuffer");
  MTL::Library* library = Repository::Shaders::readLibrary(device, config->shaderPath + "Raytracing");

	
	MTL::Function* normalFn = gBufferLib->newFunction(EXP::nsString("compute_normal_buffer"));	
	this->_kernelFn = library->newFunction(EXP::nsString("computeKernel"));

	this->_normalBuffer = Renderer::State::Compute(device, normalFn);
  this->_raytrace = Renderer::State::Compute(device, _kernelFn);

  this->_vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);

  CGRect frame = ViewAdapter::bounds();
  this->_gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  this->_resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

  this->_threadGroupSize = calcGridsize();
	
  buildModels(device);
  buildAccelerationStructures(device);

	/**
	MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(
			MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB, 
			2000, 
			1400, 
			false
	);
	device->newTexture(textureDescriptor);
	**/
}

void EXP::RayTraceLayer::buildModels(MTL::Device* device) {
	
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "f16/f16", "f16");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "sphere/sphere", "sphere1");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "sphere/sphere", "sphere2");
	
	EXP::Model* f16 = EXP::SCENE::getModel("f16");
	EXP::Model* sphere1 = EXP::SCENE::getModel("sphere1");
	EXP::Model* sphere2 = EXP::SCENE::getModel("sphere2");

	f16->move({0.0f, 0.0f, 0.0f});
	sphere1->setEmissive(true)->setColor({10.0f, 10.0f, 0.0f, 0.0f})->scale(.3f)->move({-.2f, .5f, .3f});
	sphere2->setColor({0.0f, 1.0f, 0.0f, 1.0f})->scale(.3f)->move({-.2f, .5f, -.3f});

	EXP::SCENE::buildBindlessScene(device);
	EXP::SCENE::getCamera()->setIsometric();
}

MTL::Size EXP::RayTraceLayer::calcGridsize() {
  auto threadGroupWidth = _raytrace->threadExecutionWidth();
  auto threadGroupHeight = _raytrace->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
	DEBUG("Thread group width x height: " + std::to_string(threadGroupWidth) + " x " + std::to_string(threadGroupHeight));
  return MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);
}

void EXP::RayTraceLayer::buildAccelerationStructures(MTL::Device* device) {
	_dispatchEvent = device->newEvent();	
	_buildEvent = device->newEvent();
  _primitiveDescriptors = Renderer::Descriptor::primitives(
			EXP::SCENE::getModels(), 
			_vertexDescriptor->layouts()->object(0)->stride(),
			_vertexDescriptor->layouts()->object(1)->stride()
	);
  MTL::AccelerationStructureSizes sizes = Renderer::Acceleration::sizes(device, _primitiveDescriptors);
  _heap = Renderer::Heap::primitives(device, sizes);
  _primitiveAccStructures = Renderer::Acceleration::primitives(device, _heap, queue, _primitiveDescriptors, sizes, _buildEvent);
  _instanceDescriptor = Renderer::Descriptor::instance(device, _primitiveAccStructures, EXP::SCENE::getModels());
  _instanceAccStructure = Renderer::Acceleration::instance(device, queue, _instanceDescriptor, _buildEvent);
}

void EXP::RayTraceLayer::rebuildAccelerationStructures(MTK::View* view) {
  _instanceDescriptor = Renderer::Descriptor::instance(view->device(), _primitiveAccStructures, EXP::SCENE::getModels());
	_instanceAccStructure = Renderer::Acceleration::instance(view->device(), queue, _instanceDescriptor, _buildEvent);
}

void EXP::RayTraceLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* notUsed) {
	
	// Update camera part of the bindless scene
	EXP::SCENE::updateBindlessScene(view->device());

	// Scene action
	if (IO::isPressed(KEY_T)) { 
		for (Model* model : EXP::SCENE::getModels()) {
			model->rotate(EXP::MATH::yRotation(-1.0f));
		}
	}
	rebuildAccelerationStructures(view);

	// Render pass 1 
  MTL::CommandBuffer* normalBuffer = queue->commandBuffer();
	MTL::ComputePassDescriptor* computePassDescriptor1 = MTL::ComputePassDescriptor::alloc()->init();
	MTL::ComputeCommandEncoder* normalEncoder = normalBuffer->computeCommandEncoder(computePassDescriptor1);
	normalEncoder->setComputePipelineState(this->_normalBuffer);
	normalEncoder->dispatchThreads(_gridSize, _threadGroupSize);
	normalEncoder->endEncoding();
	normalBuffer->encodeSignalEvent(_dispatchEvent, 1);
	normalBuffer->commit();
	
	// Render pass 2
	CA::MetalDrawable* drawable = view->currentDrawable();
  MTL::Texture* texture = drawable->texture();

	MTL::CommandBuffer* buffer = queue->commandBuffer();
	MTL::ComputePassDescriptor* computePassDescriptor2 = MTL::ComputePassDescriptor::alloc()->init();
	MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder(computePassDescriptor2);
  
	encoder->setComputePipelineState(_raytrace);

  encoder->setTexture(view->currentDrawable()->texture(), 0);
	
	encoder->useHeap(_heap);
  encoder->setAccelerationStructure(_instanceAccStructure, 1);
	
	for (MTL::Resource* resource : EXP::SCENE::getResources()) {
		encoder->useResource(resource, MTL::ResourceUsageRead);
  }
	encoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);

  encoder->dispatchThreads(_gridSize, _threadGroupSize);
  encoder->endEncoding();

  buffer->presentDrawable(drawable);
  buffer->commit();

	}

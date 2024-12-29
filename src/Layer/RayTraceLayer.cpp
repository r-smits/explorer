#include "Math/Transformation.h"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLCommandEncoder.hpp"
#include "Metal/MTLComputePass.hpp"
#include "Renderer/Types.h"
#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>


EXP::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {
	
	MTL::Library* gbufferLib = Repository::Shaders::readLibrary(device, config->shaderPath + "GBuffer");
	MTL::Library* temporalReuseLib = Repository::Shaders::readLibrary(device, config->shaderPath + "Temporal"); 
  MTL::Library* raytraceLib = Repository::Shaders::readLibrary(device, config->shaderPath + "Raytracing");

	MTL::Function* gbufferFn = gbufferLib->newFunction(EXP::nsString("g_buffer"));
	MTL::Function* temporalReuseFn = temporalReuseLib->newFunction(EXP::nsString("temporal_reuse"));
	this->_kernelFn = raytraceLib->newFunction(EXP::nsString("computeKernel"));

	this->_gbufferState = Renderer::State::Compute(device, gbufferFn);
	this->_temporalReuseState = Renderer::State::Compute(device, temporalReuseFn);
  this->_raytraceState = Renderer::State::Compute(device, _kernelFn);

  this->_vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);

  CGRect frame = ViewAdapter::bounds();
  this->_gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  this->_resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

  this->_threadGroupSize = calcGridsize();
	
  buildModels(device);
  buildAccelerationStructures(device);
}

void EXP::RayTraceLayer::buildModels(MTL::Device* device) {

	// EXP::SCENE::addTexture(device, "wpositions", Renderer::TextureAccess::READ_WRITE);
	// EXP::SCENE::addTexture(device, "wnormals", Renderer::TextureAccess::READ_WRITE);
	// EXP::SCENE::addTexture(device, "colors", Renderer::TextureAccess::READ_WRITE);
	
	EXP::SCENE::addTexture(device, "reservoirs", Renderer::TextureAccess::READ_WRITE);
	
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "f16/f16", "f16");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "sphere/sphere", "sphere1");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->meshPath + "sphere/sphere", "sphere2");
	
	EXP::Model* f16 = EXP::SCENE::getModel("f16");
	EXP::Model* sphere1 = EXP::SCENE::getModel("sphere1");
	EXP::Model* sphere2 = EXP::SCENE::getModel("sphere2");

	f16->move({0.0f, 0.0f, 0.0f});
	
	sphere1->setEmissive(true)->setColor({10.0f, 10.0f, 3.0f, .0f})->scale(.2f)->move({-.3f, .6f, .1f});
	sphere2->setColor({0.0f, 1.0f, 0.0f, 1.0f})->scale(.25f)->move({-.2f, .3f, -.3f});

	EXP::SCENE::buildBindlessScene(device);
	EXP::SCENE::getCamera()->setIsometric();
}

MTL::Size EXP::RayTraceLayer::calcGridsize() {
  auto threadGroupWidth = _raytraceState->threadExecutionWidth();
  auto threadGroupHeight = _raytraceState->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
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

	// Scene action & update acceleration structure
	if (IO::isPressed(KEY_T)) { 
		for (Model* model : EXP::SCENE::getModels()) {
			model->rotate(EXP::MATH::yRotation(-1.0f));
		}
	}
	rebuildAccelerationStructures(view);


	// ------------------------------ //
	// GBuffer												//
	// ------------------------------ //
	/**	
	MTL::CommandBuffer* gbufferCommand = queue->commandBuffer();
	MTL::ComputePassDescriptor* gbufferDescriptor = MTL::ComputePassDescriptor::alloc()->init();
	MTL::ComputeCommandEncoder* gbufferEncoder = gbufferCommand->computeCommandEncoder(gbufferDescriptor);
	
	gbufferEncoder->setComputePipelineState(this->_gbufferState);
	gbufferEncoder->setTexture(view->currentDrawable()->texture(), 0);
	
	gbufferEncoder->useHeap(_heap);
	gbufferEncoder->setAccelerationStructure(_instanceAccStructure, 1);	
	gbufferEncoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);

	for (MTL::Resource* resource : EXP::SCENE::getResources()) {
		gbufferEncoder->useResource(resource, MTL::ResourceUsageWrite);
  }

	gbufferEncoder->dispatchThreads(_gridSize, _threadGroupSize);
	gbufferEncoder->endEncoding();

	// gbufferCommand->presentDrawable(view->currentDrawable());
	gbufferCommand->commit();
	**/
	
	// ------------------------------ //
	// Temporal Re-use								//
	// ------------------------------ //
	MTL::CommandBuffer* temporalCommand = queue->commandBuffer();
	MTL::ComputePassDescriptor* temporalDescriptor = MTL::ComputePassDescriptor::alloc()->init();
	MTL::ComputeCommandEncoder* temporalEncoder = temporalCommand->computeCommandEncoder(temporalDescriptor);
	
	temporalEncoder->setComputePipelineState(this->_temporalReuseState);
	temporalEncoder->setTexture(view->currentDrawable()->texture(), 0);
	
	temporalEncoder->useHeap(_heap);
	temporalEncoder->setAccelerationStructure(_instanceAccStructure, 1);	
	
	for (MTL::Resource* resource : EXP::SCENE::getResources()) {
		temporalEncoder->useResource(resource, MTL::ResourceUsageRead);
  }
	
	temporalEncoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);

	temporalEncoder->dispatchThreads(_gridSize, _threadGroupSize);
	temporalEncoder->endEncoding();

	temporalCommand->presentDrawable(view->currentDrawable());
	temporalCommand->commit();

	
	// ------------------------------ //
	// RT encoding										//
	// ------------------------------ //
	/**	
	MTL::CommandBuffer* buffer = queue->commandBuffer();
	MTL::ComputePassDescriptor* computePassDescriptor2 = MTL::ComputePassDescriptor::alloc()->init();
	MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder(computePassDescriptor2);
  
	encoder->setComputePipelineState(this->_raytraceState);
  encoder->setTexture(view->currentDrawable()->texture(), 0);
	
	encoder->useHeap(_heap);
  encoder->setAccelerationStructure(_instanceAccStructure, 1);
	
	for (MTL::Resource* resource : EXP::SCENE::getResources()) {
		encoder->useResource(resource, MTL::ResourceUsageRead);
  }
	
	encoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);

  encoder->dispatchThreads(_gridSize, _threadGroupSize);
  encoder->endEncoding();

  buffer->presentDrawable(view->currentDrawable());
  buffer->commit();
	**/

}

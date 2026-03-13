#include "Math/Transformation.h"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLCommandEncoder.hpp"
#include "Metal/MTLComputePass.hpp"
#include "Renderer/Types.h"
#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>


using s_repo = Repository::Shaders;
using r_acc = Renderer::Acceleration;


EXP::RayTraceLayer::RayTraceLayer(MTL::Device* device, std::shared_ptr<const AppProperties> _config)
    : Layer(device->retain(), _config), queue(device->newCommandQueue()) {

	MTL::Library* gbufferLib = s_repo::readLibrary(device, config->shader_path / "GBuffer");
	MTL::Library* temporalReuseLib = s_repo::readLibrary(device, config->shader_path / "RESTIR"); 
	MTL::Function* gbufferFn = gbufferLib->newFunction(EXP::nsString("g_buffer"));
	MTL::Function* temporalReuseFn = temporalReuseLib->newFunction(EXP::nsString("temporal_reuse"));

	_gbufferState = Renderer::State::Compute(device, gbufferFn);
	_temporalReuseState = Renderer::State::Compute(device, temporalReuseFn);

	_vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);
	CGRect frame = ViewAdapter::bounds();
	_gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
	_resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

	_threadGroupSize = calcGridsize(_temporalReuseState);
	_temporalDescriptor = MTL::ComputePassDescriptor::alloc()->init();
	_temporalDescriptor->retain();
		
	buildModels(device);
	buildAccelerationStructures(device);
}

void EXP::RayTraceLayer::buildModels(MTL::Device* device) {

	EXP::SCENE::addTexture(device, "reservoirs", Renderer::TextureAccess::READ_WRITE);
	
	EXP::SCENE::addModel(device, _vertexDescriptor, config->mesh_path / "f16/f16", "f16");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->mesh_path / "sphere/sphere", "sphere1");
	EXP::SCENE::addModel(device, _vertexDescriptor, config->mesh_path / "sphere/sphere", "sphere2");
	
	EXP::Model* f16 = EXP::SCENE::getModel("f16");
	EXP::Model* sphere1 = EXP::SCENE::getModel("sphere1");
	EXP::Model* sphere2 = EXP::SCENE::getModel("sphere2");

	f16->move({0.0f, 0.0f, 0.0f});
	
	sphere1->setEmissive(true)->setColor({4.0f, 4.0f, 1.f, .0f})->scale(.2f)->move({-.3f, .6f, .1f});
	sphere2->setColor({0.0f, 1.0f, 0.0f, 1.0f})->scale(.25f)->move({-.2f, .3f, -.3f});

	EXP::SCENE::buildBindlessScene(device);
	EXP::SCENE::getCamera()->setIsometric();
}

MTL::Size EXP::RayTraceLayer::calcGridsize(const MTL::ComputePipelineState* state) {
  auto threadGroupWidth = state->threadExecutionWidth();
  auto threadGroupHeight = state->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  DEBUG("Thread group width x height: " + std::to_string(threadGroupWidth) + " x " + std::to_string(threadGroupHeight));
  return MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);
}

void EXP::RayTraceLayer::buildAccelerationStructures(MTL::Device* device) {
	// events to wait for while building
	_dispatchEvent = device->newEvent();	
	_buildEvent = device->newEvent();

	// primitive acc structures
	int vStride = _vertexDescriptor->layouts()->object(0)->stride();
	int pStride = _vertexDescriptor->layouts()->object(1)->stride();
	_primitiveDescriptors = Renderer::Descriptor::primitives(EXP::SCENE::getMeshes(), vStride, pStride);
	MTL::AccelerationStructureSizes primitiveSizes = Renderer::Acceleration::sizes(device, _primitiveDescriptors);
	_heap = Renderer::Heap::primitives(device, primitiveSizes);
	_primitiveAccStructures = Renderer::Acceleration::primitives(device, _heap, queue, _primitiveDescriptors, primitiveSizes, _buildEvent);
	
	// instance acc structure
	_instanceDescriptor = Renderer::Descriptor::instance(device, _primitiveAccStructures, EXP::SCENE::getMeshes())->retain();
	_instanceSizes = device->accelerationStructureSizes(_instanceDescriptor);
	_scratchBuffer = device->newBuffer(_instanceSizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate)->retain();
	_instanceAccStructure = device->newAccelerationStructure(_instanceSizes.accelerationStructureSize);
  	_instanceAccStructure = Renderer::Acceleration::instance(device, queue, _instanceAccStructure, _instanceDescriptor, _scratchBuffer, _buildEvent);
}

void EXP::RayTraceLayer::rebuildAccelerationStructures(MTK::View* view) {
    _instanceDescriptor = Renderer::Descriptor::updateTransformationMatrix(EXP::SCENE::getMeshes(), _instanceDescriptor);
    _instanceAccStructure = Renderer::Acceleration::instance(device, queue, _instanceAccStructure, _instanceDescriptor, _scratchBuffer, _buildEvent);
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
	// Temporal Re-use RESTIR GI	  //
	// ------------------------------ //
	MTL::CommandBuffer* temporalCommand = queue->commandBuffer();
	temporalCommand->encodeWait(_buildEvent, 2);
	MTL::ComputeCommandEncoder* temporalEncoder = temporalCommand->computeCommandEncoder(_temporalDescriptor);
	
	temporalEncoder->setComputePipelineState(_temporalReuseState);
	temporalEncoder->setTexture(view->currentDrawable()->texture(), 0);
	
	temporalEncoder->useHeap(_heap);
	temporalEncoder->setAccelerationStructure(_instanceAccStructure, 1);
	temporalEncoder->useResource(_instanceAccStructure, MTL::ResourceUsageRead);
	
	const std::vector<MTL::Resource*>& resources = EXP::SCENE::getResources();
	temporalEncoder->useResources(resources.data(), resources.size(), MTL::ResourceUsageRead | MTL::ResourceUsageSample);
	temporalEncoder->setBuffer(EXP::SCENE::getBindlessScene(), 0, 2);
	temporalEncoder->dispatchThreads(_gridSize, _threadGroupSize);
	temporalEncoder->endEncoding();

	temporalCommand->presentDrawable(view->currentDrawable());
	temporalCommand->commit();

}

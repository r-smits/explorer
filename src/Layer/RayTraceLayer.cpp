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
  this->_camera = VCamera();
	this->_camera.setIsometric();

  CGRect frame = ViewAdapter::bounds();
  this->_gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  this->_resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

  this->_threadGroupSize = calcGridsize();
	
  buildModels(device);
  buildAccelerationStructures(device);
  buildBindlessScene(device, scene);


	MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(
			MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB, 
			2000, 
			1400, 
			false
	);
	device->newTexture(textureDescriptor);
}

void EXP::RayTraceLayer::buildModels(MTL::Device* device) {

  Model* f16 = Repository::Meshes::read2(
			device, 
			_vertexDescriptor, 
			config->meshPath + "f16/f16"
	);

	Model* sphere1 = Repository::Meshes::read2(
			device, 
			_vertexDescriptor, 
			config->meshPath + "sphere/sphere"
	);

	Model* sphere2 = Repository::Meshes::read2(
			device,
			_vertexDescriptor,
			config->meshPath + "sphere/sphere"
	);

	f16->move({0.0f, 0.0f, 0.0f});
	sphere1->setEmissive(true)->setColor({10.0f, 10.0f, 0.0f, 0.0f})->scale(.3f)->move({-.2f, .5f, .3f});
	sphere2->setColor({0.0f, 1.0f, 0.0f, 1.0f})->scale(.3f)->move({-.2f, .5f, -.3f});
	
	scene.emplace_back(f16);
	scene.emplace_back(sphere1);
	scene.emplace_back(sphere2);
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
			scene, 
			_vertexDescriptor->layouts()->object(0)->stride(),
			_vertexDescriptor->layouts()->object(1)->stride()
	);
  MTL::AccelerationStructureSizes sizes = Renderer::Acceleration::sizes(device, _primitiveDescriptors);
  _heap = Renderer::Heap::primitives(device, sizes);
  _primitiveAccStructures = Renderer::Acceleration::primitives(device, _heap, queue, _primitiveDescriptors, sizes, _buildEvent);
  _instanceDescriptor = Renderer::Descriptor::instance(device, _primitiveAccStructures, scene);
  _instanceAccStructure = Renderer::Acceleration::instance(device, queue, _instanceDescriptor, _buildEvent);
}

void EXP::RayTraceLayer::rebuildAccelerationStructures(MTK::View* view) {
  _instanceDescriptor = Renderer::Descriptor::instance(view->device(), _primitiveAccStructures, scene);
	_instanceAccStructure = Renderer::Acceleration::instance(view->device(), queue, _instanceDescriptor, _buildEvent);
}

void EXP::RayTraceLayer::buildBindlessScene(MTL::Device* device, const std::vector<Model*>& scene) {
  std::vector<MTL::Resource*> resources;
	
	int totalMeshCount = 0;
	std::vector<Mesh*> allMeshes;
	for (Model* model : scene) {
		totalMeshCount += model->meshCount;
		for (Mesh* mesh : model->meshes) allMeshes.emplace_back(mesh);
	}

  int meshBufferSize = sizeof(Renderer::Mesh) * totalMeshCount;
  MTL::Buffer* meshBuffer = device->newBuffer(meshBufferSize, MTL::ResourceStorageModeShared);
  resources.emplace_back(meshBuffer);

  for (int j = 0; j < totalMeshCount; j++) {
    Renderer::Mesh* gpuMesh = ((Renderer::Mesh*)meshBuffer->contents()) + j;
    EXP::Mesh* cpuMesh = allMeshes[j];

    gpuMesh->vertices = cpuMesh->buffers[0]->gpuAddress() + cpuMesh->offsets[0];
    gpuMesh->attributes = cpuMesh->buffers[1]->gpuAddress() + cpuMesh->offsets[1];

    resources.emplace_back(cpuMesh->buffers[0]);
    resources.emplace_back(cpuMesh->buffers[1]);

    int submeshBufferSize = sizeof(Renderer::Submesh) * cpuMesh->count;
    MTL::Buffer* submeshBuffer = device->newBuffer(submeshBufferSize, MTL::ResourceStorageModeShared);
    resources.emplace_back(submeshBuffer);

    for (int k = 0; k < cpuMesh->count; k++) {
      Renderer::Submesh* gpuSubmesh = ((Renderer::Submesh*)submeshBuffer->contents()) + k;
			EXP::Submesh* cpuSubmesh = cpuMesh->submeshes()[k];

      gpuSubmesh->indices = cpuSubmesh->indexBuffer->gpuAddress() + cpuSubmesh->offset;
      gpuSubmesh->texture = cpuSubmesh->textures[0]->gpuResourceID();
			gpuSubmesh->textured = cpuSubmesh->material.useColor;
			gpuSubmesh->emissive = cpuSubmesh->material.useLight;

      resources.emplace_back(cpuSubmesh->indexBuffer);
      resources.emplace_back(cpuSubmesh->textures[0]);
    }

    gpuMesh->submeshes = submeshBuffer->gpuAddress();
  }

  int sceneBufferSize = sizeof(struct Renderer::Scene);
  MTL::Buffer* sceneBuffer = device->newBuffer(sceneBufferSize, MTL::ResourceStorageModeShared);
  resources.emplace_back(sceneBuffer);

  struct Renderer::Scene* gpuScene = ((struct Renderer::Scene*)sceneBuffer->contents());
  gpuScene->meshes = meshBuffer->gpuAddress();

  _resources = resources;
  _sceneBuffer = sceneBuffer;
}

void EXP::RayTraceLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* notUsed) {
	
	// Scene action
	if (IO::isPressed(KEY_T)) { 
		for (Model* model : scene) {
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
  encoder->setBytes(&_camera.update(), sizeof(Renderer::VCamera), 1);
	
	for (auto resource : _resources) {
		encoder->useResource(resource, MTL::ResourceUsageRead);
  }
	encoder->useHeap(_heap);
  encoder->setAccelerationStructure(_instanceAccStructure, 2);
	
	// Bindless 
	encoder->setBuffer(_sceneBuffer, 0, 3);

  encoder->dispatchThreads(_gridSize, _threadGroupSize);
  encoder->endEncoding();

  buffer->presentDrawable(drawable);
  buffer->commit();

	}

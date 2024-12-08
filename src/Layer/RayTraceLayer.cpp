#include "Math/Transformation.h"
#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>

Explorer::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {

  MTL::Library* library =
      Repository::Shaders::readLibrary(device, config->shaderPath + "Raytracing");
	
	this->_kernelFn = library->newFunction(Explorer::nsString("computeKernel"));
  this->_raytrace = Renderer::State::Compute(device, _kernelFn);

  this->_vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);
  this->_camera = VCamera();
	this->_camera.Iso();

  CGRect frame = ViewAdapter::bounds();
  this->_gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  this->_resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

  this->_threadGroupSize = calcGridsize();

  buildModels(device);
  buildAccelerationStructures(device);
  buildBindlessScene(device, scene);
}

void Explorer::RayTraceLayer::buildModels(MTL::Device* device) {
  // Setting up objects
  _lightDir = {0.0f, -1.0f, -1.0f};

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
	sphere1->setEmissive(true)->setColor({1.0f, 1.0f, 0.0f, 0.0f})->scale(.3f)->move({-.2f, .5f, .3f});
	sphere2->setColor({0.0f, 1.0f, 0.0f, 1.0f})->scale(.3f)->move({-.2f, .5f, -.3f});
	
	scene.emplace_back(f16);
	scene.emplace_back(sphere1);
	scene.emplace_back(sphere2);
}

MTL::Size Explorer::RayTraceLayer::calcGridsize() {
  auto threadGroupWidth = _raytrace->threadExecutionWidth();
  auto threadGroupHeight = _raytrace->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  return MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);
}

void Explorer::RayTraceLayer::buildAccelerationStructures(MTL::Device* device) {
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

void Explorer::RayTraceLayer::rebuildAccelerationStructures(MTL::Device* device) {
  _instanceDescriptor = Renderer::Descriptor::instance(device, _primitiveAccStructures, scene);
	_instanceAccStructure = Renderer::Acceleration::instance(device, queue, _instanceDescriptor, _buildEvent);
}

void Explorer::RayTraceLayer::buildBindlessScene(MTL::Device* device, const std::vector<Model*>& scene) {
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
    Explorer::Mesh* cpuMesh = allMeshes[j];

    gpuMesh->vertices = cpuMesh->buffers[0]->gpuAddress() + cpuMesh->offsets[0];
    gpuMesh->attributes = cpuMesh->buffers[1]->gpuAddress() + cpuMesh->offsets[1];

    resources.emplace_back(cpuMesh->buffers[0]);
    resources.emplace_back(cpuMesh->buffers[1]);

    int submeshBufferSize = sizeof(Renderer::Submesh) * cpuMesh->count;
    MTL::Buffer* submeshBuffer = device->newBuffer(submeshBufferSize, MTL::ResourceStorageModeShared);
    resources.emplace_back(submeshBuffer);

    for (int k = 0; k < cpuMesh->count; k++) {
      Renderer::Submesh* gpuSubmesh = ((Renderer::Submesh*)submeshBuffer->contents()) + k;
			Explorer::Submesh* cpuSubmesh = cpuMesh->submeshes()[k];

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

void Explorer::RayTraceLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* notUsed) {
  
	MTL::CommandBuffer* buffer = queue->commandBuffer();
  MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder();

  encoder->setComputePipelineState(_raytrace);

  CA::MetalDrawable* drawable = view->currentDrawable();
  MTL::Texture* texture = drawable->texture();
  encoder->setTexture(texture, 0);

  encoder->setBytes(&_resolution, sizeof(_resolution), 0);
  encoder->setBytes(&_lightDir, sizeof(_lightDir), 1);

  Renderer::RTTransform transform = _camera.update();
  encoder->setBytes(&transform, sizeof(transform), 2);
	
	if (IO::isPressed(KEY_T)) { 
		for (Model* model : scene) {
			model->rotate(Transformation::yRotation(-1.0f));
		}
	}

	rebuildAccelerationStructures(view->device());
	
	encoder->useHeap(_heap);
  encoder->setAccelerationStructure(_instanceAccStructure, 3);
  
  for (auto resource : _resources) {
		encoder->useResource(resource, MTL::ResourceUsageRead);
  }
	encoder->setBuffer(_sceneBuffer, 0, 4);

  encoder->dispatchThreads(_gridSize, _threadGroupSize);

  encoder->endEncoding();
  buffer->presentDrawable(drawable);
  buffer->commit();

	}

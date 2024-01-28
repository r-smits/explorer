#include <DB/Repository.hpp>
#include <Events/IOState.h>
#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>

Explorer::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {

  MTL::Library* library =
      Repository::Shaders::readLibrary(device, config->shaderPath + "Raytracing");
  _kernelFn = library->newFunction(Explorer::nsString("computeKernel"));
  _raytrace = Renderer::State::Compute(device, _kernelFn);

  _vertexDescriptor = Renderer::Descriptor::vertex(device, Renderer::Layouts::vertexNIP);
  _camera = VCamera();

  CGRect frame = ViewAdapter::bounds();
  _gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  _resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

  _threadGroupSize = calcGridsize();

  buildModels(device);
  buildAccelerationStructures(device);
  buildBindlessScene(device);

	_camera.Iso();

}

void Explorer::RayTraceLayer::buildModels(MTL::Device* device) {
  // Setting up objects
  _lightDir = {-1, -1, -1};

  Renderer::Sphere sphere1 = {
      {.5f, 0.2f, -2.0f},
      1.5f
  };
  Renderer::Sphere sphere2 = {
      {-1.0f, -0.6f, 0.5f},
      .5f
  };
  Renderer::Sphere sphere3 = {
      {0.0f, -6.0f, 0.0f},
      5.0f
  };

  Renderer::RTMaterial sphere1Mat = {
      {1.0f, 0.0f, 1.0f},
      {0.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.0f}
  };

  Renderer::RTMaterial sphere2Mat = {
      {0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.0f}
  };

  Renderer::RTMaterial sphere3Mat = {
      {0.0f, 0.0f, 1.0f},
      {0.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.0f}
  };

  _spheres[0] = sphere1;
  _spheres[1] = sphere2;
  _spheres[2] = sphere3;

  _materials[0] = sphere1Mat;
  _materials[1] = sphere2Mat;
  _materials[2] = sphere3Mat;

  _modelsarr[0] =
      Repository::Meshes::read2(device, _vertexDescriptor, config->meshPath + "f16/f16");
}

MTL::Size Explorer::RayTraceLayer::calcGridsize() {
  auto threadGroupWidth = _raytrace->threadExecutionWidth();
  auto threadGroupHeight = _raytrace->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  return MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);
}

void Explorer::RayTraceLayer::buildAccelerationStructures(MTL::Device* device) {
  MTL::Event* buildEvent = device->newEvent();
  NS::Array* primitiveDescriptors = Renderer::Descriptor::primitives(
      _modelsarr[0], _vertexDescriptor->layouts()->object(0)->stride()
  );
  MTL::AccelerationStructureSizes sizes =
      Renderer::Acceleration::sizes(device, primitiveDescriptors);
  _heap = Renderer::Heap::primitives(device, sizes);
  _primitiveAccStructures = Renderer::Acceleration::primitives(
      device, _heap, queue, primitiveDescriptors, sizes, buildEvent
  );
  MTL::InstanceAccelerationStructureDescriptor* instanceDescriptor =
      Renderer::Descriptor::instance(device, _primitiveAccStructures, 1, _modelsarr);
  _instanceAccStructure =
      Renderer::Acceleration::instance(device, queue, instanceDescriptor, buildEvent);
}

void Explorer::RayTraceLayer::buildBindlessScene(MTL::Device* device) {
  std::vector<MTL::Resource*> resources;
	
	int modelBufferSize = sizeof(struct Renderer::Model) * 1;
  MTL::Buffer* modelBuffer = device->newBuffer(modelBufferSize, MTL::ResourceStorageModeShared);
  resources.emplace_back(modelBuffer);
	for (int i = 0; i < 1; i++) {
		struct Renderer::Model* gpuModel = ((struct Renderer::Model*)modelBuffer->contents()) + i;
    Explorer::Model* cpuModel = _modelsarr[i];

		int meshBufferSize = sizeof(struct Renderer::Mesh) * 1;
		MTL::Buffer* meshBuffer = device->newBuffer(meshBufferSize, MTL::ResourceStorageModeShared);
		resources.emplace_back(meshBuffer);
		DEBUG("Bindless: built mesh buffer.");
		for (int j = 0; j < 1; j++) {
			struct Renderer::Mesh* gpuMesh = ((struct Renderer::Mesh*)meshBuffer->contents()) + j;
			Explorer::Mesh* cpuMesh = cpuModel->meshes[j];

			gpuMesh->vertices = cpuMesh->buffers[0]->gpuAddress() + cpuMesh->offsets[0];
			gpuMesh->attributes = cpuMesh->buffers[1]->gpuAddress() + cpuMesh->offsets[1];

			resources.emplace_back(cpuMesh->buffers[0]);
			resources.emplace_back(cpuMesh->buffers[1]);

			int submeshBufferSize = sizeof(struct Renderer::Submesh);
			MTL::Buffer* submeshBuffer = device->newBuffer(submeshBufferSize, MTL::ResourceStorageModeShared);
			resources.emplace_back(submeshBuffer);
			for (int k = 0; k < cpuMesh->count; k++) {
				struct Renderer::Submesh* gpuSubmesh = ((struct Renderer::Submesh*)submeshBuffer->contents()) + k;
				Explorer::Submesh* cpuSubmesh = cpuMesh->submeshes()[k];

				gpuSubmesh->indices = cpuSubmesh->indexBuffer->gpuAddress() + cpuSubmesh->offset;
				gpuSubmesh->texture = cpuSubmesh->textures[0]->gpuResourceID();
				resources.emplace_back(cpuSubmesh->indexBuffer);
				resources.emplace_back(cpuSubmesh->textures[0]);
			}
		
			gpuMesh->submeshes = submeshBuffer->gpuAddress();
		}
		gpuModel->meshes = meshBuffer->gpuAddress();	
	}

	int sceneBufferSize = sizeof(struct Renderer::Scene);
  MTL::Buffer* sceneBuffer = device->newBuffer(sceneBufferSize, MTL::ResourceStorageModeShared);
  resources.emplace_back(sceneBuffer);

  struct Renderer::Scene* gpuScene = ((struct Renderer::Scene*)sceneBuffer->contents());
  gpuScene->models = modelBuffer->gpuAddress();

  _resources = resources;
  _sceneBuffer = sceneBuffer;
}

void Explorer::RayTraceLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* notUsed) {
	t += 1.0f;
  if (t > 360) t -= 360.0f;

	//(t < 180) ? _lightDir += {0.0f, 1.0 / 90, 0.0f} : _lightDir += {0.0f, -1.0 / 90, -0.0f};
	//(t < 180) ? _lightDir += {1.0 / 90, 0.0f, 0.0f} : _lightDir += {-1.0 / 90, 0.0f, -0.0f};
	//(t < 180) ? _lightDir += {0.0f, 0.0f, 1.0 / 90} : _lightDir += {0.0f, 0.0f, -1.0 / 90};
	//DEBUG("Light dir: " + std::to_string(_lightDir.x) + " " + std::to_string(_lightDir.y) + " " + std::to_string(_lightDir.z));
	
  MTL::CommandBuffer* buffer = queue->commandBuffer();
  MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder();

  CA::MetalDrawable* drawable = view->currentDrawable();
  MTL::Texture* texture = drawable->texture();

  encoder->setComputePipelineState(_raytrace);

  encoder->setTexture(texture, 0);
  encoder->setBytes(&_resolution, sizeof(_resolution), 0);
  encoder->setBytes(&_lightDir, sizeof(_lightDir), 1);

  Renderer::RTTransform transform = _camera.update();
  encoder->setBytes(&transform, sizeof(transform), 2);
  
	//encoder->setBytes(&_spheres, sizeof(Renderer::Sphere) * 3, 3);

  //float spherecount = (float)sizeof(_spheres) / sizeof(Renderer::Sphere);
  //encoder->setBytes(&spherecount, 4, 5);

  encoder->setBytes(&_materials, sizeof(Renderer::RTMaterial) * 3, 4);

  encoder->useHeap(_heap);
  encoder->setAccelerationStructure(_instanceAccStructure, 6);

  encoder->setBuffer(_sceneBuffer, 0, 9);

  for (auto resource : _resources) {
    encoder->useResource(resource, MTL::ResourceUsageRead);
  }

  encoder->dispatchThreads(_gridSize, _threadGroupSize);

  encoder->endEncoding();
  buffer->presentDrawable(drawable);
  buffer->commit();
}

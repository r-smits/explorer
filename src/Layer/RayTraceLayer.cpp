#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>
#include <Events/IOState.h>
#include <DB/Repository.hpp>

Explorer::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {
  _raytrace = Renderer::State::compute(device, config->shaderPath + "Raytracing");
	_lightDir = {-1, -1, -1};
	
	Renderer::Sphere sphere1 = {{.5f, 0.2f,	-2.0f}, 1.5f};
	Renderer::Sphere sphere2 = {{-1.0f, -0.6f, 0.5f}, .5f};
	Renderer::Sphere sphere3 = {{0.0f, -6.0f, 0.0f}, 5.0f};
	
	Renderer::RTMaterial sphere1Mat = {{1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
	Renderer::RTMaterial sphere2Mat = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
	Renderer::RTMaterial sphere3Mat = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};

	spheres[0] = sphere1;
	spheres[1] = sphere2;
	spheres[2] = sphere3;

	materials[0] = sphere1Mat;
	materials[1] = sphere2Mat;
	materials[2] = sphere3Mat;

	f16 = Repository::Meshes::read(device, config->meshPath + "f16/f16");
  //f16->translate({0.0f, 0.0f, -2.5f});

	//Renderer::Accelerator::Accelerator::




  auto threadGroupWidth = _raytrace->threadExecutionWidth();
  auto threadGroupHeight = _raytrace->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  _threadGroupSize = MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);

  CGRect frame = ViewAdapter::bounds();
  _gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  _resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};

	_camera = VCamera();
}

void Explorer::RayTraceLayer::onUpdate(MTK::View* view, MTL::CommandBuffer* buffer) {
  //MTL::CommandBuffer* buffer = queue->commandBuffer();
  MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder();

  CA::MetalDrawable* drawable = view->currentDrawable();
  MTL::Texture* texture = drawable->texture();
  
	encoder->setComputePipelineState(_raytrace);

  encoder->setTexture(texture, 0);
  encoder->setBytes(&_resolution, sizeof(_resolution), 0);
	encoder->setBytes(&_lightDir, sizeof(_lightDir), 1);

	Renderer::RTTransform transform = _camera.update();
	encoder->setBytes(&transform, sizeof(transform), 2);
	encoder->setBytes(&spheres, sizeof(Renderer::Sphere) * 3, 3);
	float spherecount = (float) sizeof(spheres) / sizeof(Renderer::Sphere);
	
	encoder->setBytes(&spherecount, 4, 5);

	encoder->setBytes(&materials, sizeof(Renderer::RTMaterial) * 3, 4);

  encoder->dispatchThreads(_gridSize, _threadGroupSize);
	
  encoder->endEncoding();
  //buffer->presentDrawable(drawable);
  //buffer->commit();
}

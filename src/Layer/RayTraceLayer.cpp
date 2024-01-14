#include <Layer/RayTraceLayer.h>
#include <Renderer/Renderer.h>
#include <Events/IOState.h>

Explorer::RayTraceLayer::RayTraceLayer(MTL::Device* device, AppProperties* config)
    : Layer(device->retain(), config), queue(device->newCommandQueue()) {
  _raytrace = Renderer::State::compute(device, config->shaderPath + "Raytracing");
	_lightDir = {-1, -1, 1};

  auto threadGroupWidth = _raytrace->threadExecutionWidth();
  auto threadGroupHeight = _raytrace->maxTotalThreadsPerThreadgroup() / threadGroupWidth;
  _threadGroupSize = MTL::Size::Make(threadGroupWidth, threadGroupHeight, 1);

  CGRect frame = ViewAdapter::bounds();
  _gridSize = MTL::Size::Make(frame.size.width * 2, frame.size.height * 2, 1);
  _resolution = {(float)_gridSize.width, (float)_gridSize.height, (float)_gridSize.depth};
}

void Explorer::RayTraceLayer::onUpdate(MTK::View* view, MTL::RenderCommandEncoder* notUsed) {
  MTL::CommandBuffer* buffer = queue->commandBuffer();
  MTL::ComputeCommandEncoder* encoder = buffer->computeCommandEncoder();

  CA::MetalDrawable* drawable = view->currentDrawable();
  MTL::Texture* texture = drawable->texture();
  
	encoder->setComputePipelineState(_raytrace);
  encoder->setTexture(texture, 0);
  encoder->setBytes(&_resolution, sizeof(_resolution), 0);
	
	if (IO::isPressed(ARROW_LEFT)) _lightDir.z += -.01;
	if (IO::isPressed(ARROW_RIGHT)) _lightDir.z += .01;

	encoder->setBytes(&_lightDir, sizeof(_lightDir), 1);
  encoder->dispatchThreads(_gridSize, _threadGroupSize);
	
  encoder->endEncoding();
  buffer->presentDrawable(drawable);
  buffer->commit();
}

#include <DB/Repository.hpp>
#include <Renderer/State.h>

MTL::RenderPipelineState*
Renderer::State::render(MTL::Device* device, MTL::RenderPipelineDescriptor* descriptor) {
  NS::Error* error = nullptr;
  MTL::RenderPipelineState* state = device->newRenderPipelineState(descriptor, &error);
  if (!state) Explorer::printError(error);
  error->release();
  descriptor->release();
  DEBUG("Generated pipeline state.");
  return state;
}

MTL::ComputePipelineState*
Renderer::State::compute(MTL::Device* device, std::string path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::Function* kernel = library->newFunction(Explorer::nsString("computeKernel"));
  NS::Error* error = nullptr;
	MTL::ComputePipelineState* state = device->newComputePipelineState(kernel, &error);
  if (!state) Explorer::printError(error);
	return state;
}

MTL::DepthStencilState* Renderer::State::depthStencil(MTL::Device* device) {
  MTL::DepthStencilDescriptor* descriptor = MTL::DepthStencilDescriptor::alloc()->init();
  descriptor->setDepthWriteEnabled(true);
  descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
  return device->newDepthStencilState(descriptor);
}

MTL::SamplerState* Renderer::State::sampler(MTL::Device* device) {
  MTL::SamplerDescriptor* descriptor = MTL::SamplerDescriptor::alloc()->init();
  descriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
  descriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
  return device->newSamplerState(descriptor);
}

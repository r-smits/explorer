#include <Renderer/State.h>


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

#include "Metal/MTLResource.hpp"
#include <Renderer/Buffer.h>
#include <stdint.h>

MTL::Buffer* Explorer::Buffer::create(MTL::Device* device, Vertex vertices[], uint32_t count) {
  auto size = count * sizeof(Vertex);
  MTL::Buffer* buffer = device->newBuffer(vertices, size, MTL::ResourceStorageModeShared);
  memcpy(buffer->contents(), vertices, size);
  return buffer;
}

MTL::Buffer* Explorer::Buffer::create(MTL::Device* device, ushort indices[], uint32_t count) {
  auto size = count * sizeof(ushort);
  MTL::Buffer* buffer = device->newBuffer(indices, size, MTL::ResourceStorageModeShared);
  memcpy(buffer->contents(), indices, size);
  return buffer;
}

MTL::Buffer* Explorer::Buffer::create(MTL::Device* device, Light* light) {
  auto size = sizeof(Light);
  MTL::Buffer* buffer = device->newBuffer(light, size, MTL::ResourceStorageModeShared);
	memcpy(buffer->contents(), light, size);
	return buffer;
}

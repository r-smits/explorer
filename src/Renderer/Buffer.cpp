#include "Metal/MTLResource.hpp"
#include "Renderer/Types.h"
#include <Renderer/Buffer.h>
#include <cstdint>
#include <stdint.h>

MTL::Buffer* Renderer::Buffer::create(MTL::Device* device, Vertex vertices[], uint32_t count) {
  auto size = count * sizeof(Vertex);
  MTL::Buffer* buffer = device->newBuffer(vertices, size, MTL::ResourceStorageModeShared);
  memcpy(buffer->contents(), vertices, size);
  return buffer;
}

MTL::Buffer* Renderer::Buffer::create(MTL::Device* device, ushort indices[], uint32_t count) {
  auto size = count * sizeof(ushort);
  MTL::Buffer* buffer = device->newBuffer(indices, size, MTL::ResourceStorageModeShared);
  memcpy(buffer->contents(), indices, size);
  return buffer;
}

MTL::Buffer* Renderer::Buffer::create(MTL::Device* device, Light* light) {
  auto size = sizeof(Light);
  MTL::Buffer* buffer = device->newBuffer(light, size, MTL::ResourceStorageModeShared);
	memcpy(buffer->contents(), light, size);
	return buffer;
}

// Add per primitive data to the buffer by reference
MTL::Buffer* Renderer::Buffer::perPrimitive(
		MTL::Device* device, 
		MTL::Buffer* vertexAttribBuffer,
		MTL::Buffer* indices,
		const int& indexCount,
		const uint8_t& txindex
) {
	
	int perPrimitiveBufferSize = sizeof(Renderer::PrimitiveAttributes) * indexCount / 3;
	MTL::Buffer* perPrimitiveBuffer = device->newBuffer(perPrimitiveBufferSize, MTL::ResourceStorageModeShared);

	Renderer::PrimitiveAttributes* primAttribPtr = (Renderer::PrimitiveAttributes*)perPrimitiveBuffer->contents();
	Renderer::VertexAttributes* verAttribPtr = (Renderer::VertexAttributes*)vertexAttribBuffer->contents();
	uint32_t* indexPtr = (uint32_t*)indices->contents();
  	
	for (int i = 0; i < indexCount / 3; i += 1) {

		uint32_t vertAttribIndex1 = *(indexPtr + i * 3 + 0);
		uint32_t vertAttribIndex2 = *(indexPtr + i * 3 + 1);
		uint32_t vertAttribIndex3 = *(indexPtr + i * 3 + 2);

		Renderer::VertexAttributes* vertAttrib1 = verAttribPtr + vertAttribIndex1;
		Renderer::VertexAttributes* vertAttrib2 = verAttribPtr + vertAttribIndex2;
		Renderer::VertexAttributes* vertAttrib3 = verAttribPtr + vertAttribIndex3;
		
		Renderer::PrimitiveAttributes* primAttrib = primAttribPtr + i;

		primAttrib->color[0] = vertAttrib1->color;
		primAttrib->color[1] = vertAttrib2->color;
		primAttrib->color[2] = vertAttrib3->color;

		primAttrib->normal[0] = vertAttrib1->normal;
		primAttrib->normal[1] = vertAttrib2->normal;
		primAttrib->normal[2] = vertAttrib3->normal;

		primAttrib->txcoord[0] = vertAttrib1->texture;
		primAttrib->txcoord[1] = vertAttrib2->texture;
		primAttrib->txcoord[2] = vertAttrib3->texture;
		
		primAttrib->txindex = txindex;
	}
	return perPrimitiveBuffer;
};


	
//primAttrib->txindex = txindex;

// const uint8_t& txindex


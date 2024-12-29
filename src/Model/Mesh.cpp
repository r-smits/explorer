#include "Renderer/Types.h"
#include <Model/Mesh.h>

EXP::MDL::Mesh::Mesh(
    const std::vector<MTL::Buffer*>& buffers,
    const std::vector<int>& offsets,
    const int& bufferCount,
    const std::string& name,
    const int& vertexCount
)
    : buffers(buffers), offsets(offsets), bufferCount(bufferCount), name(name),
      vertexCount(vertexCount), count(0) {
  DEBUG("Mesh: '" + this->name + "', vertex count: " + std::to_string(this->vertexCount));
}

EXP::MDL::Mesh::Mesh(EXP::MDL::Submesh* submesh, const std::string& name, const int& vertexCount)
    : name(name), vertexCount(vertexCount) {
  submeshes.emplace_back(submesh);
  if (!count) count = 0;
  count += 1;
}

EXP::MDL::Mesh::~Mesh() {
  for (EXP::MDL::Submesh* subMesh : submeshes) {
    delete subMesh;
  }
  for (MTL::Buffer* buffer : buffers) {
    buffer->release();
  }
}

const void EXP::MDL::Mesh::addSubmesh(EXP::MDL::Submesh* submesh) {
  submeshes.emplace_back(submesh);
  count += 1;
}

const void EXP::MDL::Mesh::addSubmeshes(const std::vector<EXP::MDL::Submesh*>& submeshes) {
  for (EXP::MDL::Submesh* submesh : submeshes)
    addSubmesh(submesh);
}

const std::vector<EXP::MDL::Submesh*>& EXP::MDL::Mesh::getSubmeshes() { return submeshes; }

const void EXP::MDL::Mesh::setColor(const simd::float4& color) {
  for (EXP::MDL::Submesh* submesh : this->submeshes) {
    submesh->setColor(color);
  }

	EXP::print(color);
	
	Renderer::VertexAttributes* verAttribPtr = (Renderer::VertexAttributes*)this->buffers[1]->contents();
		
	DEBUG("Verifying whether size of buffer is equal to expected vertex count.");
	assert (this->vertexCount == this->buffers[1]->length() / sizeof(Renderer::VertexAttributes));
  	
	for (int i = 0; i < this->vertexCount; i += 1) {
		Renderer::VertexAttributes* vertAttrib1 = verAttribPtr + i;
		vertAttrib1->color = {color.x, color.y, color.z, color.w};
	}

	EXP::print((verAttribPtr + this->vertexCount-1)->color);
}

/**
EXP::MDL::Mesh* EXP::MDL::Mesh::f4x4() {
	this->orientation = EXP::MATH::translation(position) * rotation * EXP::MATH::scale(factor);
  return this;
}

EXP::MDL::Mesh* EXP::MDL::Mesh::translate(const simd::float3& pos) {
	this->position += position;
  return this;
}

EXP::MDL::Mesh* EXP::MDL::Mesh::scale(const float& factor) {
	this->factor = factor;
  return this;
}

EXP::MDL::Mesh* EXP::MDL::Mesh::rotate(const simd::float4x4& rotation) {
	this->rotation = rotation;
  return this;
}
**/

const simd::float4x4& EXP::MDL::Mesh::getRotation() {
	return this->rotation;
}

const simd::float3& EXP::MDL::Mesh::getPosition() {
	return this->position;
}

/**
const simd::float4x4& EXP::MDL::Mesh::get() {
	return this->orientation;
}
**/


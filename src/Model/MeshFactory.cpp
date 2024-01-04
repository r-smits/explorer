#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Renderer/Types.h>

Explorer::LightSource::LightSource() {}

Explorer::LightSource::LightSource(MTL::Buffer* buffer) : lightBuffer(buffer) {}
Explorer::Mesh::Mesh(MTL::Buffer* vertexBuffer, MTL::Buffer* indexBuffer)
    : vertexBuffer(vertexBuffer), indexBuffer(indexBuffer), factor(1.0f),
      mRotate(Transformation::zRotation(0)) {
  trans = {0.0f, 0.0f, 0.0f};
}

Explorer::Mesh::Mesh() : mRotate(Transformation::zRotation(0)), factor(1.0f) {
  trans = {0.0f, 0.0f, 0.0f};
}

MTL::Buffer* Explorer::MeshFactory::triangle(MTL::Device* device) {
  Vertex vertices[3] = {
      {{-0.75, -0.75, 0.0f, 1.0f}, {1.0, 0.0, 0.0}},
      { {0.75, -0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}},
      {     {0, 0.75, 0.0f, 1.0f}, {0.0, 0.0, 1.0}}
  };
  return Buffer::create(device, vertices, 3);
}

Explorer::Mesh* Explorer::Mesh::scale(float fFactor) {
  factor = fFactor;
  return this;
}
Explorer::Mesh* Explorer::Mesh::rotate(simd::float4x4 rotation) {
  mRotate = rotation;
  return this;
}
Explorer::Mesh* Explorer::Mesh::translate(simd::float3 fTrans) {
  trans = fTrans;
  return this;
}

void Explorer::Mesh::printDebug() const {
  std::stringstream ss;
  ss << "Factor: " << factor << "Translation: " << trans[0] << ", " << trans[1] << ", " << trans[2];
  DEBUG(ss.str());
}

simd::float4x4 Explorer::Mesh::transform() {
  return Transformation::translation(trans) * mRotate * Transformation::scale(factor);
}

Explorer::Mesh* Explorer::MeshFactory::pyramid(MTL::Device* device) {
  Vertex vertices[4] = {
      {  {0.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0}},
      { {1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
  };

  ushort indices[12] = {0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 2, 3};

  return new Mesh(
      Buffer::create(device, vertices, 4), Buffer::create(device, indices, 12)
  );
};

Explorer::Mesh* Explorer::MeshFactory::quad(MTL::Device* device) {
  Vertex vertices[4] = {
      {{-0.75, -0.75, 0.0f, 1.0f}, {1.0, 0.0, 0.0}},
      { {0.75, -0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}},
      {  {0.75, 0.75, 0.0f, 1.0f}, {0.0, 0.0, 1.0}},
      { {-0.75, 0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}}
  };
  ushort indices[6] = {0, 1, 2, 2, 3, 0};
  return new Mesh(
      Buffer::create(device, vertices, 4), Buffer::create(device, indices, 6)
  );
}

Explorer::Mesh* Explorer::MeshFactory::cube(MTL::Device* device) {
  Vertex vertices[8] = {
      {  {-1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f}},
      { {-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, 0.0f}},
      {   {1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}},
      {  {1.0f, -1.0f, 1.0f, 1.0f},  {0.5f, 0.5f, 1.0f}},
      { {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.25f}},
      {  {1.0f, 1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f}},
      {{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}},
      { {1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}}
  };

  ushort indices[36] = {0, 1, 2, 2, 1, 3, 5, 2, 3, 5, 3, 7, 0, 2, 4, 2, 5, 4,
                        0, 1, 4, 4, 1, 6, 5, 4, 6, 5, 6, 7, 3, 1, 6, 3, 6, 7};
  return new Mesh(
      Buffer::create(device, vertices, 8), Buffer::create(device, indices, 36)
  );
}

Explorer::LightSource* Explorer::MeshFactory::light(MTL::Device *device) {
	Light light = {{1.0f, 0.0f, 0.0f, 1.0f}};
	return new LightSource(Buffer::create(device, &light));
}

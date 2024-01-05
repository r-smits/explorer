#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Renderer/Types.h>

Explorer::Object::Object() : position(simd::float3(0)), rotation(simd::float4x4(1)), scale(1) {}

simd::float4x4 Explorer::Object::f4x4() {
  return Transformation::translation(position) * rotation * Transformation::scale(scale);
}

Explorer::LightSource::LightSource() {}

Explorer::LightSource::LightSource(MTL::Buffer* buffer) : lightBuffer(buffer) {}

Explorer::Mesh::Mesh(MTL::Buffer* vertexBuffer, MTL::Buffer* indexBuffer)
    : vertexBuffer(vertexBuffer), indexBuffer(indexBuffer) {}

Explorer::Mesh::Mesh() {}

MTL::Buffer* Explorer::MeshFactory::triangle(MTL::Device* device) {
  Vertex vertices[3] = {
      {{-0.75, -0.75, 0.0f, 1.0f}, {1.0, 0.0, 0.0}},
      { {0.75, -0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}},
      {     {0, 0.75, 0.0f, 1.0f}, {0.0, 0.0, 1.0}}
  };
  return Buffer::create(device, vertices, 3);
}

Explorer::Mesh* Explorer::MeshFactory::pyramid(MTL::Device* device) {
  Vertex vertices[4] = {
      {  {0.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0}},
      { {1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
  };

  ushort indices[12] = {0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 2, 3};

  return new Mesh(Buffer::create(device, vertices, 4), Buffer::create(device, indices, 12));
};

Explorer::Mesh* Explorer::MeshFactory::quad(MTL::Device* device) {
  Vertex vertices[4] = {
      {{-0.75, -0.75, 0.0f, 1.0f}, {1.0, 0.0, 0.0}},
      { {0.75, -0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}},
      {  {0.75, 0.75, 0.0f, 1.0f}, {0.0, 0.0, 1.0}},
      { {-0.75, 0.75, 0.0f, 1.0f}, {0.0, 1.0, 0.0}}
  };
  ushort indices[6] = {0, 1, 2, 2, 3, 0};
  return new Mesh(Buffer::create(device, vertices, 4), Buffer::create(device, indices, 6));
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
  return new Mesh(Buffer::create(device, vertices, 8), Buffer::create(device, indices, 36));
}

Explorer::LightSource* Explorer::MeshFactory::light(MTL::Device* device) {
  Light light = {
      {1.0f, 0.0f, 0.0f, 1.0f}
  };
  return new LightSource(Buffer::create(device, &light));
}

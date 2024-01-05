#include <DB/Repository.h>
#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Renderer/Types.h>

Explorer::Object::Object() : position(simd::float3(0)), rotation(simd::float4x4(1)), scale(1) {}

// Computation of matrices are down right -> left.
// Meaning you first need to translate, then rotate, then scale
simd::float4x4 Explorer::Object::f4x4() {
  return Transformation::translation(position) * rotation * Transformation::scale(scale);
}

Explorer::LightSource::LightSource(MTL::Buffer* buffer) : lightBuffer(buffer) {}

Explorer::Mesh::Mesh(MTL::Buffer* vertexBuffer, MTL::Texture* texture)
    : vertexBuffer(vertexBuffer), texture(texture) {}

Explorer::Mesh* Explorer::MeshFactory::pyramid(MTL::Device* device, std::string texture) {
  Renderer::Vertex vertices[4] = {
      {  {0.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0}},
      { {1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
  };

  ushort indices[12] = {0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 2, 3};

  Submesh* submesh = new Submesh(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      12,
      MTL::IndexType::IndexTypeUInt16,
      Renderer::Buffer::create(device, indices, 12),
      0
  );

  Mesh* pyramid = new Mesh(
      Renderer::Buffer::create(device, vertices, 4), Repository::Textures::read(device, texture)
  );
  pyramid->add(submesh);
  return pyramid;
};

Explorer::Mesh* Explorer::MeshFactory::cube(MTL::Device* device, std::string texture) {
  Renderer::Vertex vertices[8] = {
      {  {-1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f}, {0, 0}},
      { {-1.0f, -1.0f, 1.0f, 1.0f},  {1.0f, 1.0f, 0.0f}, {0, 0}},
      {   {1.0f, 1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0, 0}},
      {  {1.0f, -1.0f, 1.0f, 1.0f},  {0.5f, 0.5f, 1.0f}, {0, 0}},
      { {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.25f}, {0, 0}},
      {  {1.0f, 1.0f, -1.0f, 1.0f},  {0.0f, 0.0f, 1.0f}, {0, 0}},
      {{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}, {0, 0}},
      { {1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0f}, {0, 0}}
  };

  ushort indices[36] = {0, 1, 2, 2, 1, 3, 5, 2, 3, 5, 3, 7, 0, 2, 4, 2, 5, 4,
                        0, 1, 4, 4, 1, 6, 5, 4, 6, 5, 6, 7, 3, 1, 6, 3, 6, 7};

  Submesh* submesh = new Submesh(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      36,
      MTL::IndexType::IndexTypeUInt16,
      Renderer::Buffer::create(device, indices, 36),
      0
  );

  Mesh* cube = new Mesh(
      Renderer::Buffer::create(device, vertices, 8), Repository::Textures::read(device, texture)
  );

  cube->add(submesh);
  return cube;
}

Explorer::LightSource* Explorer::MeshFactory::light(MTL::Device* device) {
  Renderer::Light light = {
      {700.0f, 700.0f, 0.0f, 1.0f}
  };
  return new LightSource(Renderer::Buffer::create(device, &light));
}

Explorer::Mesh* Explorer::MeshFactory::quad(MTL::Device* device, std::string texture) {

  Renderer::Vertex vertices[4] = {
      {{-1.0f, -1.0f, 0.0f, 1.0f}, {1.0, 0.0, 0.0}, {0, 0}},
      { {1.0f, -1.0f, 0.0f, 1.0f}, {0.0, 1.0, 0.0}, {1, 0}},
      {  {1.0f, 1.0f, 0.0f, 1.0f}, {0.0, 0.0, 1.0}, {1, 1}},
      { {-1.0f, 1.0f, 0.0f, 1.0f}, {0.0, 1.0, 0.0}, {0, 1}}
  };
  ushort indices[6] = {0, 1, 2, 2, 3, 0};

  Submesh* submesh = new Submesh(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      6,
      MTL::IndexType::IndexTypeUInt16,
      Renderer::Buffer::create(device, indices, 6),
      0
  );

  Mesh* quad = new Mesh(
      Renderer::Buffer::create(device, vertices, 4), Repository::Textures::read(device, texture)
  );

  quad->add(submesh);
  return quad;
}

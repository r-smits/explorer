#include <DB/Repository.hpp>
#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Renderer/Types.h>
#include <View/ViewAdapter.hpp>

Explorer::Object::Object()
    : orientation(simd::float4x4(1)), position(simd::float3(0)), rotation(simd::float4x4(1)),
      factor(1) {}

// Computation of matrices are down right -> left.
// Meaning you first need to translate, then rotate, then scale
Explorer::Object* Explorer::Object::f4x4() {
  orientation = Transformation::translation(position) * rotation * Transformation::scale(factor);
  return this;
}
Explorer::Object* Explorer::Object::translate(simd::float3 position) {
  this->position += position;
  return this;
}

Explorer::Object* Explorer::Object::scale(float factor) {
  this->factor = factor;
  return this;
}
Explorer::Object* Explorer::Object::rotate(simd::float4x4 rotation) {
  this->rotation = rotation;
  return this;
}

Explorer::Mesh::Mesh(const Submesh& submesh, const std::string& name, const int& vertexCount)
    : vertexBufferOffset(0), name(name), vertexCount(vertexCount) {
  vSubmeshes.emplace_back(submesh);
  if (!count) count = 0;
  count += 1;
}

Explorer::Mesh::Mesh(
    MTL::Buffer* vertexBuffer,
    const int vertexBufferOffset,
    const std::string& name,
    const int& vertexCount
)
    : vertexBuffer(vertexBuffer), vertexBufferOffset(vertexBufferOffset), name(name),
      vertexCount(vertexCount), count(0) {}

Explorer::Model* Explorer::MeshFactory::pyramid(MTL::Device* device, std::string texture) {

  return nullptr;

  /**
Renderer::Vertex vertices[4] = {
{  {0.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
{{-1.0f, -1.0f, -1.0f, 1.0f},  {0.0f, 1.0f, 0.0}},
{ {1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
{   {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
};

ushort indices[12] = {0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 2, 3};

Submesh* submesh = new Submesh(
{
    {1.0f, 1.0f, 1.0f, 1.0f},
    true
},
Repository::Textures::read(device, texture),
MTL::PrimitiveType::PrimitiveTypeTriangle,
12,
MTL::IndexType::IndexTypeUInt16,
Renderer::Buffer::create(device, indices, 12),
0
);

Mesh* pyramid = new Mesh(Renderer::Buffer::create(device, vertices, 4), 0,
"Pyramid", 4); pyramid->add(submesh); return new Model(pyramid);
  **/
};

Explorer::Model* Explorer::MeshFactory::cube(MTL::Device* device, std::string texture) {

  return nullptr;
  /**
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
{
    {1.0f, 1.0f, 1.0f, 1.0f},
    true
},
Repository::Textures::read(device, texture),
MTL::PrimitiveType::PrimitiveTypeTriangle,
36,
MTL::IndexType::IndexTypeUInt16,
Renderer::Buffer::create(device, indices, 36),
0
);

Mesh* cube = new Mesh(Renderer::Buffer::create(device, vertices, 8), 0, "Cube",
8); cube->add(submesh); return new Model(cube);
  **/
}

Explorer::Model* Explorer::MeshFactory::quad(MTL::Device* device, std::string texture) {

  return nullptr;

  /**
Renderer::Vertex vertices[4] = {
{{-1.0f, -1.0f, 0.0f, 1.0f}, {1.0, 0.0, 0.0}, {0, 0}},
{ {1.0f, -1.0f, 0.0f, 1.0f}, {0.0, 1.0, 0.0}, {1, 0}},
{  {1.0f, 1.0f, 0.0f, 1.0f}, {0.0, 0.0, 1.0}, {1, 1}},
{ {-1.0f, 1.0f, 0.0f, 1.0f}, {0.0, 1.0, 0.0}, {0, 1}}
};
ushort indices[6] = {0, 1, 2, 2, 3, 0};

Submesh* submesh = new Submesh(
{
    {1.0f, 1.0f, 1.0f, 1.0f},
    true
},
Repository::Textures::read(device, texture),
MTL::PrimitiveType::PrimitiveTypeTriangle,
6,
MTL::IndexType::IndexTypeUInt16,
Renderer::Buffer::create(device, indices, 6),
0
);

Mesh* quad = new Mesh(Renderer::Buffer::create(device, vertices, 4), 0, "Quad",
4);

quad->add(submesh);
return new Model(quad);
  **/
}

Explorer::Light* Explorer::Light::translate(simd::float3 pos) {
  position += pos;
  data.position = position;
  // data.position = convert();
  return this;
}

// Convert from vertex plane to fragment plane
simd::float3 Explorer::Light::convert() {
  return {origin.x + (origin.x * position.x), origin.y - (origin.y * position.y), position.z};
}

Explorer::Light* Explorer::MeshFactory::light(MTL::Device* device) {
  CGRect frame = ViewAdapter::bounds();
  Renderer::Light data = {
      {(float)frame.size.width, (float)frame.size.height, 0.0f}, // position (x, y, z)
      {1.0f, 1.0f, 1.0f}, // color (r, g, b)
      {1.0f, 1.0f, 1.0f, 1.0f}  // brightness, fAmbient, fDiffuse, fSpecular
  };
  return new Explorer::Light(data);
}

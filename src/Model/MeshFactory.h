#pragma once
#include <Math/Transformation.h>
#include <Renderer/Types.h>
#include <View/ViewAdapter.hpp>
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {

struct Object {
public:
  Object();
  ~Object(){};

public:
  virtual Object* f4x4();
  virtual Object* translate(simd::float3 pos);
  virtual Object* scale(float factor);
  virtual Object* rotate(simd::float4x4 rotation);
  virtual simd::float4x4 get() { return orientation; }

public:
  simd::float4x4 rotation;

public:
  simd::float4x4 orientation;
  simd::float3 position;
  float factor;
};

struct Submesh {

  Submesh(
      Renderer::Material material,
      MTL::Texture* texture,
      MTL::PrimitiveType primitiveType,
      int indexCount,
      MTL::IndexType indexType,
      MTL::Buffer* indexBuffer,
      int offset
  )
      : material(material), texture(texture), primitiveType(primitiveType), indexCount(indexCount),
        indexType(indexType), indexBuffer(indexBuffer), offset(offset) {}
  ~Submesh() {
    indexBuffer->release();
    texture->release();
  }

  Renderer::Material material;
  MTL::Texture* texture;
  MTL::PrimitiveType primitiveType;
  NS::UInteger indexCount;
  MTL::IndexType indexType;
  MTL::Buffer* indexBuffer;
  NS::UInteger offset;
};

struct Mesh : public Object {
public:
  Mesh(MTL::Buffer* vertexBuffer);
  Mesh(Submesh* submesh) { submeshes.push_back(submesh); };
  ~Mesh() {
    for (Submesh* subMesh : submeshes) {
      delete subMesh;
    }
    vertexBuffer->release();
  };

public:
  void add(Submesh* submesh) { submeshes.push_back(submesh); }

public:
  MTL::Buffer* vertexBuffer;
  std::vector<Submesh*> submeshes;
};

struct Model : public Object {
public:
  Model(std::vector<Mesh*> meshes) : meshes(meshes) {}
  Model(Mesh* mesh) { meshes.push_back(mesh); }
  ~Model() {
    for (Mesh* mesh : meshes) {
      delete mesh;
    };
  }

public:
  std::vector<Mesh*> meshes;
};

struct Light : public Object {

  Light(Renderer::Light data) : data(data) {
		DEBUG("Light :: Getting bounds inside of initializer ...");
    CGRect frame = ViewAdapter::bounds();
    origin = {(float)frame.size.width, (float)frame.size.height, 0};
		DEBUG("Light :: Initializer done");
  };
  ~Light(){};

public:
  virtual Light* translate(simd::float3 pos) override;
  virtual simd::float3 convert();

public:
  Renderer::Light data;
  simd::float3 origin;
};

struct Lights : Object {

public:
  Lights(Renderer::Light light, MTL::Buffer* buffer) : lightBuffer(buffer) {
    array.push_back(light);
  };
  ~Lights() { lightBuffer->release(); };

public:
  std::vector<Renderer::Light> array;

  // LightSource(MTL::Buffer* fragmentBuffer);

public:
  MTL::Buffer* lightBuffer;
};

class MeshFactory {

public:
  static Model* pyramid(MTL::Device* device, std::string texture);
  static Model* quad(MTL::Device* device, std::string texture);
  static Model* cube(MTL::Device* device, std::string texture);
  static Light* light(MTL::Device* device);
};

} // namespace Explorer

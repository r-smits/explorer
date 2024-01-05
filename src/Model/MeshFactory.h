#pragma once
#include <Renderer/Types.h>
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {

struct Object {
public:
  Object();
  ~Object(){};

public:
  virtual simd::float4x4 f4x4();

public:
  simd::float3 position;
  simd::float4x4 rotation;
  float scale;
};

struct Submesh {

  Submesh(
      MTL::PrimitiveType primitiveType,
      int indexCount,
      MTL::IndexType indexType,
      MTL::Buffer* indexBuffer,
      int offset
  )
      : primitiveType(primitiveType), indexCount(indexCount), indexType(indexType),
        indexBuffer(indexBuffer), offset(offset) {}
  ~Submesh() { indexBuffer->release(); }

  MTL::PrimitiveType primitiveType;
  NS::UInteger indexCount;
  MTL::IndexType indexType;
  MTL::Buffer* indexBuffer;
  NS::UInteger offset;
};

struct Mesh : public Object {
public:
  Mesh(MTL::Buffer* vertexBuffer, MTL::Texture* texture);
  Mesh(){};
  ~Mesh() {
    for (Submesh* subMesh : submeshes) {
      delete subMesh;
    }
    vertexBuffer->release();
    texture->release();
  };

public:
  void add(Submesh* submesh) { submeshes.push_back(submesh); }

public:
  MTL::Buffer* vertexBuffer;
  std::vector<Submesh*> submeshes;
  MTL::Texture* texture;
};

struct LightSource : public Object {

public:
  LightSource(){};
  ~LightSource() { lightBuffer->release(); };
  LightSource(MTL::Buffer* fragmentBuffer);

public:
  MTL::Buffer* lightBuffer;
};

class MeshFactory {

public:
  static Mesh* pyramid(MTL::Device* device, std::string texture);
  static Mesh* quad(MTL::Device* device, std::string texture);
  static Mesh* cube(MTL::Device* device, std::string texture);
  static LightSource* light(MTL::Device* device);
};

} // namespace Explorer

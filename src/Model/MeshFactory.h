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
      std::vector<MTL::Texture*> textures,
      MTL::PrimitiveType primitiveType,
      int indexCount,
      MTL::IndexType indexType,
      MTL::Buffer* indexBuffer,
      int offset
  )
      : material(material), textures(textures), primitiveType(primitiveType), indexCount(indexCount),
        indexType(indexType), indexBuffer(indexBuffer), offset(offset) {}
  ~Submesh() {
    indexBuffer->release();
    for (MTL::Texture* texture : textures)
      texture->release();
  }

  Renderer::Material material;
  std::vector<MTL::Texture*> textures;
  MTL::PrimitiveType primitiveType;
  NS::UInteger indexCount;
  MTL::IndexType indexType;
  MTL::Buffer* indexBuffer;
  NS::UInteger offset;
};

struct Mesh : public Object {
public:
  Mesh(
      MTL::Buffer* vertexBuffer,
      const int vertexBufferOffset,
      const std::string& name = "Mesh",
      const int& vertexCount = -1
  );
  Mesh(const Submesh& submesh, const std::string& name = "Mesh", const int& vertexCount = -1);
  ~Mesh() {
    vertexBuffer->release();
  };

public:
  void add_all(std::vector<Submesh> submeshes) {
    for (Submesh submesh : submeshes)
      add(submesh);
  }
  void add(const Submesh& submesh) {
    vSubmeshes.emplace_back(submesh);
    count += 1;
  }
  std::vector<Submesh> submeshes() const { return vSubmeshes; }

public:
  MTL::Buffer* vertexBuffer;
  const int vertexBufferOffset;
  int count;

private:
  std::vector<Submesh> vSubmeshes;

private:
  const std::string& name;
  const int& vertexCount;
};

struct Model : public Object {
public:
  Model(std::vector<Mesh> meshes, const std::string& name = "Model", const int& vertexCount = -1)
      : name(name), vertexCount(vertexCount), meshes(meshes) {}
  Model(const Mesh& mesh, const std::string& name = "Model") : name(name) { meshes.push_back(mesh); }
  ~Model() {}
  
public:
  std::string toString() {
    std::stringstream ss;
    ss << "(Model: " << name << ", vertices: " << vertexCount << ")";
    return ss.str();
  }

private:
  const std::string& name;
  int vertexCount;

public:
  std::vector<Mesh> meshes;
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

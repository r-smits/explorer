#pragma once
#include <Math/Transformation.h>
#include <Renderer/Types.h>
#include <View/ViewAdapter.hpp>
#include <pch.h>
#include <simd/simd.h>
#include <Model/Submesh.h>
#include <Model/Mesh.h>
#include <Model/Object.h>

namespace EXP {


struct Model {
public:
  Model(
      const std::vector<EXP::MDL::Mesh*>& meshes,
      const std::string& name = "Model",
      const int& vertexCount = -1
  )
      : name(name), vertexCount(vertexCount) {
    addMeshes(meshes);
  }
  Model(EXP::MDL::Mesh* mesh) { meshes.push_back(mesh); }

  void addMesh(EXP::MDL::Mesh* mesh) {
    meshes.emplace_back(mesh);
    meshCount += 1;
  }

  void addMeshes(const std::vector<EXP::MDL::Mesh*>& meshes) {
    for (EXP::MDL::Mesh* mesh : meshes)
      addMesh(mesh);
  }

  EXP::Model* rotate(const simd::float4x4& rotation) {
    for (EXP::MDL::Mesh* mesh : meshes) {
      mesh->rotate(rotation * mesh->getRotation());
    }
    return this;
  }

  EXP::Model* scale(const float& scalar) {
    for (EXP::MDL::Mesh* mesh : meshes) {
      mesh->scale(scalar);
    }
    return this;
  }

  EXP::Model* move(const simd::float3& vec) {
    for (EXP::MDL::Mesh* mesh : meshes) {
      mesh->translate(vec);
    }
    return this;
  }

  EXP::Model* f4x4() {
    for (EXP::MDL::Mesh* mesh : meshes) {
      mesh->f4x4();
    }
    return this;
  }

  EXP::Model* setColor(const simd::float4& color) {
    for (EXP::MDL::Mesh* mesh : this->meshes) {
      mesh->setColor(color);
    }
    return this;
  }

  EXP::Model* setEmissive(const bool& emissive) {
    for (EXP::MDL::Mesh* mesh : this->meshes) {
      for (EXP::MDL::Submesh* submesh : mesh->getSubmeshes()) {
				submesh->setEmissive(true);
      }
    }
    return this;
  }

  const simd::float4x4& get() { 
		return this->meshes[0]->get();
	}

  ~Model() {
    for (EXP::MDL::Mesh* mesh : meshes) {
      delete mesh;
    }
  }

public:
  std::string toString() {
    std::stringstream ss;
    ss << "(Model: " << name << ", vertices: " << vertexCount << ")";
    return ss.str();
  }

public:
  std::string name;
  int vertexCount;

public:
  std::vector<EXP::MDL::Mesh*> meshes;
  int meshCount;
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
  Light* translate(const simd::float3& pos);
  virtual simd::float3 convert();

public:
  Renderer::Light data;
  simd::float3 origin;
};

struct Lights : public Object {

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

} // namespace EXP

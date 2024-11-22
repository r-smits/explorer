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
  virtual Object* translate(const simd::float3& pos);
  virtual Object* scale(const float& factor);
  virtual Object* rotate(const simd::float4x4& rotation);
  virtual const simd::float4x4& get() { return orientation; }

public:
  simd::float4x4 rotation;

public:
  simd::float4x4 orientation;
  simd::float3 position;
  float factor;
};

struct Submesh {
public:
  Renderer::Material material;
  std::vector<MTL::Texture*> textures;
  MTL::PrimitiveType primitiveType;
  NS::UInteger indexCount;
  MTL::IndexType indexType;
  MTL::Buffer* indexBuffer;
  NS::UInteger offset;

  Submesh(
      Renderer::Material material,
      std::vector<MTL::Texture*> textures,
      MTL::PrimitiveType primitiveType,
      int indexCount,
      MTL::IndexType indexType,
      MTL::Buffer* indexBuffer,
      int offset
  )
      : material(material), textures(textures), primitiveType(primitiveType),
        indexCount(indexCount), indexType(indexType), indexBuffer(indexBuffer), offset(offset) {}
  ~Submesh() {
    indexBuffer->release();
    for (auto texture : textures)
      texture->release();
  }
};

struct Mesh : public Object {
public:
  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
  int bufferCount;
  int count;
	
private:
  std::vector<Submesh*> vSubmeshes;
  std::string name;
  int vertexCount;

public:
  Mesh(
      std::vector<MTL::Buffer*> buffers,
      std::vector<int> offsets,
      const int& bufferCount,
      const std::string& name = "Mesh",
      const int& vertexCount = -1
  );
  Mesh(Submesh* submesh, std::string name = "Mesh", int vertexCount = -1);
  ~Mesh() {
    for (Submesh* subMesh : vSubmeshes) { delete subMesh; }
    for (MTL::Buffer* buffer : buffers) { buffer->release(); }
  };

public:
  void add_all(std::vector<Submesh*> submeshes) {
    for (Submesh* submesh : submeshes)
      add(submesh);
  }
  void add(Submesh* submesh) {
    vSubmeshes.emplace_back(submesh);
    count += 1;
  }
	void setColor(const simd::float4& color) {
		for (int i = 0; i < this->vertexCount; i += 1) {
			Renderer::VertexAttributes* attributes = (Renderer::VertexAttributes*) buffers[1]->contents() + i;
			attributes->color = color;
		}
	}

  std::vector<Submesh*> submeshes() { return vSubmeshes; }
};

struct Model {
public:
  Model(const std::vector<Mesh*>& meshes, std::string name = "Model", int vertexCount = -1)
      : name(name), vertexCount(vertexCount) {
    add(meshes);
  }
  Model(Mesh* mesh) { meshes.push_back(mesh); }

  void add(Mesh* mesh) {
    meshes.emplace_back(mesh);
    meshCount += 1;
  }

  void add(const std::vector<Mesh*>& meshes) {
    for (Mesh* mesh : meshes)
      add(mesh);
  }

	Explorer::Model* rotate(const simd::float4x4& rotation) {
		for (Mesh* mesh : meshes) {
			mesh->rotate(rotation * mesh->rotation);
		}
		return this;
	}

	Explorer::Model* scale(const float& scalar) {
		for (Mesh* mesh: meshes) {
			mesh->scale(scalar);	
		}
		return this;
	}

	Explorer::Model* move(const simd::float3& vec) {
		for (Mesh* mesh: meshes) {
			mesh->translate(vec);
		}
		return this;
	}

	Explorer::Model* f4x4() {	
		for (Mesh* mesh: meshes) {
			mesh->f4x4();
		}
		return this;
	}
	
	Explorer::Model* setColor(const simd::float4& color) {
		for (Mesh* mesh : this->meshes) {
			mesh->setColor(color);
		}
		return this;
	}

	Explorer::Model* setEmissive(const bool& emissive) {
		for (Mesh* mesh : this->meshes) {
			for (Submesh* submesh : mesh->submeshes()) {
				submesh->material.useLight = emissive;
			}
		}
		return this;
	}

	const simd::float4x4& get() {
		return this->meshes[0]->get();
	}

  ~Model() {
    for (Mesh* mesh : meshes) {
      delete mesh;
    }
  }

public:
  std::string toString() {
    std::stringstream ss;
    ss << "(Model: " << name << ", vertices: " << vertexCount << ")";
    return ss.str();
  }

private:
  std::string name;
  int vertexCount;

public:
  std::vector<Mesh*> meshes;
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
  virtual Light* translate(const simd::float3& pos) override;
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

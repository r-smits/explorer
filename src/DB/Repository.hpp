#pragma once
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <pch.h>

namespace Repository {

struct TextureWithName {
	MTL::Texture* texture;
	std::string name;
};

class Shaders {
public:
  Shaders(){};
  ~Shaders(){};

public: // Reading/Writing
  static NS::String* open(std::string path);
  static MTL::BinaryArchive* createBinaryArchive(MTL::Device* device);
  static bool
  write(MTL::Device* device, MTL::RenderPipelineDescriptor* descriptor, std::string path);

public: // Read
  static MTL::RenderPipelineDescriptor* read(MTL::Device* device, std::string name);
  static MTL::Library* readLibrary(MTL::Device*, std::string name);
};

class Textures {
public:
  Textures(){};
  ~Textures(){};

public:
	static MTL::Texture* read(MTL::Device* device, std::string path);
};

class Meshes {
public:
  Meshes(){};
  ~Meshes(){};
  //static EXP::Model* read(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, std::string path, bool useTexture = true, bool useLight = true);
	static EXP::Model* read(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, const std::string& relativePath);
};
}; // namespace Repository

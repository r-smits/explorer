#pragma once
#include <Model/MeshFactory.h>
#include <pch.h>

namespace Repository {

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
  // Another exists but only with objective-c types.
	static MTL::Texture* read(MTL::Device* device, std::string path);
};

class Meshes {
public:
  Meshes(){};
  ~Meshes(){};
  static Explorer::Model* read(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, std::string path, bool useTexture = true, bool useLight = true);
	static Explorer::Model* read2(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, std::string path);
};
}; // namespace Repository

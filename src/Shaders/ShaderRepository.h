#pragma once
#include "Foundation/NSString.hpp"
#include <pch.h>

namespace Explorer {

class ShaderRepository {
public:
  ShaderRepository(MTL::Device* device, std::string basePath);

public: // Reading/Writing
  virtual NS::String* open(std::string path);
  virtual MTL::Library* getLibrary(std::string path);
  virtual MTL::BinaryArchive* createBinaryArchive();
  virtual bool write(MTL::RenderPipelineDescriptor* descriptor, std::string path);
  virtual MTL::RenderPipelineDescriptor* read(std::string shaderName);

private:
  const NS::StringEncoding encoding;
  MTL::Device* device;
  const std::string basePath;
};
}; // namespace Explorer

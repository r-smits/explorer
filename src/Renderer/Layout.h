#pragma once
#include "Metal/MTLStageInputOutputDescriptor.hpp"
#include <cstdint>
#include <pch.h>

namespace Renderer {
enum class ShaderDataType {
  None = 0,
  Float = MTL::VertexFormatFloat,
  Float2 = MTL::VertexFormat::VertexFormatFloat2,
  Float3 = MTL::VertexFormat::VertexFormatFloat3,
  Float4 = MTL::VertexFormat::VertexFormatFloat4,
	UInt16Index = MTL::IndexType::IndexTypeUInt16,
	UInt32Index = MTL::IndexType::IndexTypeUInt32,
  Float3x3,
  Float4x4,
  Int,
  Int2,
  Int3,
  Int4
};

static uint32_t ShaderDataTypeSize(ShaderDataType type) {
  switch (type) {
  case ShaderDataType::Float:
    return 16; // Due to padding
  case ShaderDataType::Float2:
    return 16; // Due to padding
  case ShaderDataType::Float3:
    return 16; // Due to padding
  case ShaderDataType::Float4:
    return 16;
  case ShaderDataType::Float3x3:
    return 36;
  case ShaderDataType::Float4x4:
    return 64;
  case ShaderDataType::Int:
    return 4;
  case ShaderDataType::Int2:
    return 8;
  case ShaderDataType::Int3:
    return 12;
  case ShaderDataType::Int4:
    return 16;
	case ShaderDataType::UInt16Index:
		return sizeof(uint16_t);
	case ShaderDataType::UInt32Index:
		return sizeof(uint32_t);
  default:
    ERROR("Unknown shader data type!");
    return 0;
  }
}

struct BufferElement {

  BufferElement(ShaderDataType type, const std::string& name)
      : name(name), type(type), size(ShaderDataTypeSize(type)), offset(0) {}
  std::string name;
  ShaderDataType type;
  uint32_t size;
  uint32_t offset;
};

class BufferLayout {
public:
  BufferLayout(const std::initializer_list<BufferElement>& elements) : elements(elements) {}
  inline const std::vector<BufferElement>& getElements() const { return elements; }
  BufferLayout();

private:
  std::vector<BufferElement> elements;
};

struct Layouts {
  inline static BufferLayout vertex = {
      {Renderer::ShaderDataType::Float4, "position"},
      {          ShaderDataType::Float3,    "color"},
      {          ShaderDataType::Float2,  "texture"},
      {          ShaderDataType::Float3,   "normal"},
  };
};

}; // namespace Renderer

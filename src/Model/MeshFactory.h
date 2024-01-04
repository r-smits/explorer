#pragma once
#include <Renderer/Types.h>
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {


struct Mesh {
public:
  Mesh(MTL::Buffer* vertexBuffer, MTL::Buffer* indexBuffer);
  Mesh();
  ~Mesh();

public:
  virtual Mesh* scale(float factor);
  virtual Mesh* rotate(simd::float4x4 rotation);
  virtual Mesh* translate(simd::float3 trans);
  virtual simd::float4x4 transform();

public:
  virtual void printDebug() const;

public:
  MTL::Buffer* vertexBuffer;
  MTL::Buffer* indexBuffer;

private:
  float factor;
  simd::float4x4 mRotate;
  simd::float3 trans;
};

struct LightSource {
public:
  LightSource();
  ~LightSource();
  LightSource(MTL::Buffer* fragmentBuffer);

public:
  MTL::Buffer* lightBuffer;
};

class MeshFactory {

public:
  static MTL::Buffer* triangle(MTL::Device* device);
  static Mesh* pyramid(MTL::Device* device);
  static Mesh* quad(MTL::Device* device);
  static Mesh* cube(MTL::Device* device);
  static LightSource* light(MTL::Device* device);
};

} // namespace Explorer

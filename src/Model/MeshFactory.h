#pragma once
#include <Renderer/Types.h>
#include <pch.h>
#include <simd/simd.h>

namespace Explorer {


struct Object {
public:
  Object();
  ~Object() {};

public:
	// Computation of matrices are down right -> left.
  // Meaning you first need to translate, then rotate, then scale
	virtual simd::float4x4 f4x4();

public:
	simd::float3 position;
	simd::float4x4 rotation;
	float scale;
};

struct Mesh : public Object {
public:
  Mesh(MTL::Buffer* vertexBuffer, MTL::Buffer* indexBuffer);
  Mesh();
  ~Mesh();

public:
  MTL::Buffer* vertexBuffer;
  MTL::Buffer* indexBuffer;
};

struct LightSource : public Object {

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

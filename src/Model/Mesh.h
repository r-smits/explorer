#pragma once
#include <Model/Object.h>
#include <Model/Submesh.h>
#include <Math/Transformation.h>
#include <pch.h>

namespace EXP {

namespace MDL {

struct Mesh : public Object {

public:
  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
  int bufferCount;
  int count;
  std::vector<EXP::MDL::Submesh*> submeshes;
  std::string name;
  int vertexCount;

	/**
private:
	simd::float4x4 rotation;
  simd::float4x4 orientation;
  simd::float3 position;
	simd::float1 factor;
	**/

public:
  Mesh(
      const std::vector<MTL::Buffer*>& buffers,
      const std::vector<int>& offsets,
      const int& bufferCount,
      const std::string& name = "Mesh",
      const int& vertexCount = -1
  );
  Mesh(EXP::MDL::Submesh* submesh, const std::string& name = "Mesh", const int& vertexCount = -1);
  ~Mesh();

  const void addSubmesh(EXP::MDL::Submesh* submesh);
  const void addSubmeshes(const std::vector<EXP::MDL::Submesh*>& submeshes);
  const std::vector<EXP::MDL::Submesh*>& getSubmeshes();
  const void setColor(const simd::float4& color);

	/**
	EXP::MDL::Mesh* f4x4();
  EXP::MDL::Mesh* translate(const simd::float3& pos);
  EXP::MDL::Mesh* scale(const float& factor);
  EXP::MDL::Mesh* rotate(const simd::float4x4& rotation);
	**/
	
	const simd::float3& getPosition();
	const simd::float4x4& getRotation();
  // const simd::float4x4& get();

};

}; // namespace MDL
}; // namespace EXP

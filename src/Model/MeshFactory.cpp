#include <DB/Repository.hpp>
#include <Math/Transformation.h>
#include <Model/MeshFactory.h>
#include <Renderer/Buffer.h>
#include <Renderer/Types.h>
#include <View/ViewAdapter.hpp>

EXP::Model* EXP::MeshFactory::pyramid(MTL::Device* device, std::string texture) {
  Renderer::Vertex vertices[4] = {
      {  {0.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
      {{-1.0f, -1.0f, -1.0f},  {0.0f, 1.0f, 0.0}},
      { {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
      {   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
  };

  ushort indices[12] = {0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 2, 3};
	
	std::vector<MTL::Texture*> textures;
	MTL::Texture* tex = Repository::Textures::read(device, texture);
	textures.emplace_back(tex);

	EXP::MDL::Submesh* submesh = new EXP::MDL::Submesh(
      Renderer::Buffer::create(device, indices, 12),
			nullptr,
			MTL::PrimitiveType::PrimitiveTypeTriangle,
			MTL::IndexType::IndexTypeUInt16,
      12,
			0	
  );

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
  MTL::Buffer* vertexBuffer = Renderer::Buffer::create(device, vertices, 4);

  buffers.emplace_back(vertexBuffer);
  offsets.emplace_back(0);

	EXP::MDL::Mesh* pyramid = new EXP::MDL::Mesh(buffers, offsets, 1, "Pyramid", 4);
  pyramid->addSubmesh(submesh);
  return new Model(pyramid);
};

EXP::Model* EXP::MeshFactory::cube(MTL::Device* device, std::string texture) {
  Renderer::Vertex vertices[8] = {
      {  {-1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 0.0f}, {0, 0}},
      { {-1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 0.0f}, {0, 0}},
      {   {1.0f, 1.0f, 1.0f},  {1.0f, 0.0f, 1.0f}, {0, 0}},
      {  {1.0f, -1.0f, 1.0f},  {0.5f, 0.5f, 1.0f}, {0, 0}},
      { {-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.25f}, {0, 0}},
      {  {1.0f, 1.0f, -1.0f},  {0.0f, 0.0f, 1.0f}, {0, 0}},
      {{-1.0f, -1.0f, -1.0f},  {0.0f, 1.0f, 0.0f}, {0, 0}},
      { {1.0f, -1.0f, -1.0f},  {0.0f, 1.0f, 0.0f}, {0, 0}}
  };

  ushort indices[36] = {0, 1, 2, 2, 1, 3, 5, 2, 3, 5, 3, 7, 0, 2, 4, 2, 5, 4,
                        0, 1, 4, 4, 1, 6, 5, 4, 6, 5, 6, 7, 3, 1, 6, 3, 6, 7};
	
	std::vector<MTL::Texture*> textures;
	MTL::Texture* tex = Repository::Textures::read(device, texture);
	textures.emplace_back(tex);

	EXP::MDL::Submesh* submesh = new EXP::MDL::Submesh(
      Renderer::Buffer::create(device, indices, 36),
			nullptr,
			MTL::PrimitiveType::PrimitiveTypeTriangle,
			MTL::IndexType::IndexTypeUInt16,
      36,
      0
  );

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;

  MTL::Buffer* vertexBuffer = Renderer::Buffer::create(device, vertices, 8);
  buffers.emplace_back(vertexBuffer);
  offsets.emplace_back(0);

	EXP::MDL::Mesh* cube = new EXP::MDL::Mesh(buffers, offsets, 1, "Cube", 8);
  cube->addSubmesh(submesh);
  return new Model(cube);
}

EXP::Model* EXP::MeshFactory::quad(MTL::Device* device, std::string texture) {

  Renderer::Vertex vertices[4] = {
      {{-1.0f, -1.0f, 0.0f}, {1.0, 0.0, 0.0}, {0, 0}},
      { {1.0f, -1.0f, 0.0f}, {0.0, 1.0, 0.0}, {1, 0}},
      {  {1.0f, 1.0f, 0.0f}, {0.0, 0.0, 1.0}, {1, 1}},
      { {-1.0f, 1.0f, 0.0f}, {0.0, 1.0, 0.0}, {0, 1}}
  };
  ushort indices[6] = {0, 1, 2, 2, 3, 0};

	std::vector<MTL::Texture*> textures;
	MTL::Texture* tex = Repository::Textures::read(device, texture);
	textures.emplace_back(tex);

	EXP::MDL::Submesh* submesh = new EXP::MDL::Submesh(
      Renderer::Buffer::create(device, indices, 6),
			nullptr,
			MTL::PrimitiveType::PrimitiveTypeTriangle,
      MTL::IndexType::IndexTypeUInt16,
      6,
			0
  );

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;

  MTL::Buffer* vertexBuffer = Renderer::Buffer::create(device, vertices, 4);

  buffers.emplace_back(vertexBuffer);
  offsets.emplace_back(0);

	EXP::MDL::Mesh* quad = new EXP::MDL::Mesh(buffers, offsets, 1, "Quad", 4);
  quad->addSubmesh(submesh);
  return new Model(quad);
}

EXP::Light* EXP::Light::translate(const simd::float3& pos) {
  position += pos;
  data.position = position;
  // data.position = convert();
  return this;
}

// Convert from vertex plane to fragment plane
simd::float3 EXP::Light::convert() {
  return {origin.x + (origin.x * position.x), origin.y - (origin.y * position.y), position.z};
}

EXP::Light* EXP::MeshFactory::light(MTL::Device* device) {
  CGRect frame = ViewAdapter::bounds();
  Renderer::Light data = {
      {(float)frame.size.width, (float)frame.size.height, 0.0f}, // position (x, y, z)
      {1.0f, 1.0f, 1.0f}, // color (r, g, b)
      {1.0f, 1.0f, 1.0f, 1.0f}  // brightness, fAmbient, fDiffuse, fSpecular
  };
  return new EXP::Light(data);
}

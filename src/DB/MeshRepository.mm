#include <DB/Repository.h>
#include <DB/Repository.hpp>


EXP::Submesh* buildSubmesh(MTL::Device* device, MTKSubmesh* mtkSubmesh, MDLSubmesh* mdlSubmesh) {
  Renderer::Material material = [TextureRepository readMaterial:device material:mdlSubmesh.material];
  std::vector<MTL::Texture*> textures;
	Repository::TextureWithName textureWithName = [TextureRepository read:device material:mdlSubmesh.material];
	
	const int& texindex = EXP::SCENE::addTexture(textureWithName);
	textures.emplace_back(textureWithName.texture);

	material.useColor = (!textureWithName.texture) ? false : true;
	material.useLight = false;
  EXP::Submesh* submesh = new EXP::Submesh(
      material,
      textures,
      (__bridge MTL::PrimitiveType)mtkSubmesh.primitiveType,
      mtkSubmesh.indexCount,
      (__bridge MTL::IndexType)mtkSubmesh.indexType,
      (__bridge MTL::Buffer*)mtkSubmesh.indexBuffer.buffer,
      mtkSubmesh.indexBuffer.offset,
			texindex
  );
  return submesh;
}

MDLVertexDescriptor*
buildMDLVertexDescriptor(MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor) {
  MDLVertexDescriptor* mdlVertexDescriptor =
      MTKModelIOVertexDescriptorFromMetal((__bridge MTLVertexDescriptor*)vertexDescriptor);
  [[[mdlVertexDescriptor attributes] objectAtIndex:0] setName:MDLVertexAttributePosition];
  [[[mdlVertexDescriptor attributes] objectAtIndex:1] setName:MDLVertexAttributeColor];
  [[[mdlVertexDescriptor attributes] objectAtIndex:2] setName:MDLVertexAttributeTextureCoordinate];
  [[[mdlVertexDescriptor attributes] objectAtIndex:3] setName:MDLVertexAttributeNormal];
  return mdlVertexDescriptor;
}

EXP::Mesh* buildMesh(MTL::Device* device, MDLMesh* mdlMesh, MTL::VertexDescriptor* vertexDescriptor) {
  id<MTLDevice> objcppDevice = (__bridge id<MTLDevice>)device;
	
	//[mdlMesh addNormalsWithAttributeNamed:MDLVertexAttributeNormal creaseThreshold:0.7];
  mdlMesh.vertexDescriptor = buildMDLVertexDescriptor(device, vertexDescriptor);

  NSError* err = nil;
  MTKMesh* mtkMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:objcppDevice error:&err];
  if (err) EXP::printError((__bridge NS::Error*)err);
  assert(mtkMesh.submeshes.count == mtkMesh.submeshes.count);

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
  for (int i = 0; i < mtkMesh.vertexBuffers.count; i++) {
		MTL::Buffer* buffer = (__bridge MTL::Buffer*)[mtkMesh.vertexBuffers objectAtIndex:i].buffer;
    int offset = [mtkMesh.vertexBuffers objectAtIndex:i].offset;
    buffers.emplace_back(buffer);
    offsets.emplace_back(offset);
  }
	
	EXP::Mesh* mesh = new EXP::Mesh(
      buffers,
      offsets,
      mtkMesh.vertexBuffers.count,
      [[mdlMesh name] UTF8String],
      mdlMesh.vertexCount
  );
	
	DEBUG("Submeshes: " + std::to_string(mtkMesh.submeshes.count));
  for (int i = 0; i < mtkMesh.submeshes.count; i++) {
    mesh->add(
			buildSubmesh(
				device, 
				mtkMesh.submeshes[i], 
				mdlMesh.submeshes[i]
			)
		);
  }
  return mesh;
}

std::vector<EXP::Mesh*> buildMeshes(MTL::Device* device, MDLObject* object, MTL::VertexDescriptor* vertexDescriptor) {
  std::vector<EXP::Mesh*> meshes;
  if ([object isKindOfClass:[MDLMesh class]]) {
    EXP::Mesh* mesh = buildMesh(device, (MDLMesh*)object, vertexDescriptor);
    meshes.emplace_back(mesh);
  }

  for (MDLObject* child in object.children) {
    std::vector<EXP::Mesh*> meshes = buildMeshes(device, child, vertexDescriptor);
    meshes.insert(meshes.end(), meshes.begin(), meshes.end());
  }
  return meshes;
}

EXP::Model* Repository::Meshes::read(MTL::Device* cppDevice, MTL::VertexDescriptor* vertexDescriptor, const std::string& path) {
  NSURL* url = (__bridge NSURL*)EXP::nsUrl(path + ".obj");
	id<MTLDevice> device = (__bridge id<MTLDevice>) cppDevice;
  MTKMeshBufferAllocator* bufferAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice: device];
  MDLAsset* mdlAsset = [[MDLAsset alloc] initWithURL:url vertexDescriptor:nil bufferAllocator:bufferAllocator];

  std::vector<EXP::Mesh*> allMeshes;
  for (MDLObject* mdlObject : mdlAsset) {
    std::vector<EXP::Mesh*> meshes = buildMeshes(cppDevice, mdlObject, vertexDescriptor);
		for (EXP::Mesh* mesh : meshes) { allMeshes.emplace_back(mesh); }
  }

  EXP::Model* model = new EXP::Model(allMeshes, allMeshes[0]->name, -1);
  return model;
}


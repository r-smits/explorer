#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <ModelIO/ModelIO.h>


EXP::MDL::Submesh* buildSubmesh(
	MTL::Device* device, 
	MTKSubmesh* mtkSubmesh, 
	MDLSubmesh* mdlSubmesh, 
	MTL::Buffer* vertexAttribBuffer
) {
  
	Renderer::Material material = [TextureRepository readMaterial:device material:mdlSubmesh.material];
	Repository::TextureWithName textureWithName = [TextureRepository read:device material:mdlSubmesh.material];
	const int& texindex = EXP::SCENE::addTexture(textureWithName);
	
	MTL::Buffer* indexBuffer = (__bridge MTL::Buffer*)mtkSubmesh.indexBuffer.buffer;
	MTL::Buffer* primitiveAttribBuffer = Renderer::Buffer::perPrimitive(
		device, 
		vertexAttribBuffer, 
		indexBuffer,
		mtkSubmesh.indexCount,
		texindex
	);

  EXP::MDL::Submesh* submesh = new EXP::MDL::Submesh(
      indexBuffer,
			primitiveAttribBuffer,	
			(__bridge MTL::PrimitiveType)mtkSubmesh.primitiveType,
      (__bridge MTL::IndexType)mtkSubmesh.indexType,
			mtkSubmesh.indexCount,
      mtkSubmesh.indexBuffer.offset
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

EXP::MDL::Mesh* buildMesh(MTL::Device* device, MDLMesh* mdlMesh, MTL::VertexDescriptor* vertexDescriptor) {
  id<MTLDevice> objcppDevice = (__bridge id<MTLDevice>)device;
	
	//[mdlMesh addNormalsWithAttributeNamed:MDLVertexAttributeNormal creaseThreshold:0.7];
  mdlMesh.vertexDescriptor = buildMDLVertexDescriptor(device, vertexDescriptor);

  NSError* err = nil;
  MTKMesh* mtkMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:objcppDevice error:&err];
  if (err) EXP::printError((__bridge NS::Error*)err);
  assert(mtkMesh.submeshes.count == mtkMesh.submeshes.count);

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
	
	// MDLMeshBufferTypeVertex 
	// Both buffers use the same amount of vertices -> buf[0] the points, buf[1] the attribs. So safe to use.
  for (int i = 0; i < mtkMesh.vertexBuffers.count; i++) {
		MTL::Buffer* buffer = (__bridge MTL::Buffer*)[mtkMesh.vertexBuffers objectAtIndex:i].buffer;
    int offset = [mtkMesh.vertexBuffers objectAtIndex:i].offset;
    buffers.emplace_back(buffer);
    offsets.emplace_back(offset);
  }
	
	EXP::MDL::Mesh* mesh = new EXP::MDL::Mesh(
      buffers,
      offsets,
      mtkMesh.vertexBuffers.count,
      [[mdlMesh name] UTF8String],
      mdlMesh.vertexCount
  );
	
	DEBUG("Submeshes: " + std::to_string(mtkMesh.submeshes.count));
  for (int i = 0; i < mtkMesh.submeshes.count; i++) {
    mesh->addSubmesh(
			buildSubmesh(
				device, 
				mtkMesh.submeshes[i], 
				mdlMesh.submeshes[i],
				buffers[1]
			)
		);
  }
	// buffers[1]->release();
  return mesh;
}

std::vector<EXP::MDL::Mesh*> buildMeshes(MTL::Device* device, MDLObject* object, MTL::VertexDescriptor* vertexDescriptor) {
  std::vector<EXP::MDL::Mesh*> meshes;
  if ([object isKindOfClass:[MDLMesh class]]) {
    EXP::MDL::Mesh* mesh = buildMesh(device, (MDLMesh*)object, vertexDescriptor);
    meshes.emplace_back(mesh);
  }

  for (MDLObject* child in object.children) {
    std::vector<EXP::MDL::Mesh*> meshes = buildMeshes(device, child, vertexDescriptor);
    meshes.insert(meshes.end(), meshes.begin(), meshes.end());
  }
  return meshes;
}

EXP::Model* Repository::Meshes::read(MTL::Device* cppDevice, MTL::VertexDescriptor* vertexDescriptor, const std::string& path) {
  NSURL* url = (__bridge NSURL*)EXP::nsUrl(path + ".obj");
	id<MTLDevice> device = (__bridge id<MTLDevice>) cppDevice;
  MTKMeshBufferAllocator* bufferAllocator = [[MTKMeshBufferAllocator alloc] initWithDevice: device];
  MDLAsset* mdlAsset = [[MDLAsset alloc] initWithURL:url vertexDescriptor:nil bufferAllocator:bufferAllocator];

  std::vector<EXP::MDL::Mesh*> allMeshes;
  for (MDLObject* mdlObject : mdlAsset) {
    std::vector<EXP::MDL::Mesh*> meshes = buildMeshes(cppDevice, mdlObject, vertexDescriptor);
		for (EXP::MDL::Mesh* mesh : meshes) { allMeshes.emplace_back(mesh); }
  }

  EXP::Model* model = new EXP::Model(allMeshes, allMeshes[0]->name, -1);
  return model;
}


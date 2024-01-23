#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <MetalKit/MetalKit.h>
#include <Model/MeshFactory.h>
#include <ModelIO/ModelIO.h>
#include <Renderer/Descriptor.h>

Explorer::Model* Repository::Meshes::read(
    MTL::Device* device,
    MTL::VertexDescriptor* vertexDescriptor,
    std::string path = "",
    bool useTexture,
    bool useLight
) {
  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path + ".obj");
  MDLVertexDescriptor* mdlVertexDescriptor =
      MTKModelIOVertexDescriptorFromMetal((__bridge MTLVertexDescriptor*)vertexDescriptor);

  [[[mdlVertexDescriptor attributes] objectAtIndex:0] setName:MDLVertexAttributePosition];
  [[[mdlVertexDescriptor attributes] objectAtIndex:1] setName:MDLVertexAttributeColor];
  [[[mdlVertexDescriptor attributes] objectAtIndex:2] setName:MDLVertexAttributeTextureCoordinate];
  [[[mdlVertexDescriptor attributes] objectAtIndex:3] setName:MDLVertexAttributeNormal];

  MTKMeshBufferAllocator* bufferAllocator =
      [[MTKMeshBufferAllocator alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  NSError* error = nil;
  MDLAsset* asset = [[MDLAsset alloc] initWithURL:url
                                 vertexDescriptor:mdlVertexDescriptor
                                  bufferAllocator:bufferAllocator
                                 preserveTopology:true
                                            error:&error];
  if (error) Explorer::printError((__bridge NS::Error*)error);

  [asset loadTextures];

  NSArray<MDLMesh*>* mdlMeshes = nil;
  NSArray<MTKMesh*>* meshes = [MTKMesh newMeshesFromAsset:asset
                                                   device:(__bridge id<MTLDevice>)device
                                             sourceMeshes:&mdlMeshes
                                                    error:&error];
  if (error) Explorer::printError((__bridge NS::Error*)error);

  std::vector<Explorer::Mesh*> expMeshes;
  MTL::Texture* texture = Repository::Textures::read(device, path);

  int tvCount = 0;

  int i = 0;
  for (MTKMesh* mesh : meshes) {

    MDLMesh* mdlMesh = mdlMeshes[i];
    int vertexCount = mdlMesh.vertexCount;
    tvCount += vertexCount;

    std::vector<MTL::Buffer*> buffers;
    std::vector<int> offsets;

    DEBUG("Number of buffers: " + std::to_string(mesh.vertexBuffers.count));
    for (int k = 0; k < mesh.vertexBuffers.count; k++) {
      MTL::Buffer* buffer = (__bridge MTL::Buffer*)[mesh.vertexBuffers objectAtIndex:k].buffer;
      int offset = [mesh.vertexBuffers objectAtIndex:k].offset;
      DEBUG("Found offset for buffer: " + std::to_string(offset));
      buffers.push_back(buffer);
      offsets.push_back(offset);
    }

    Explorer::Mesh* expMesh = new Explorer::Mesh(
        buffers, offsets, mesh.vertexBuffers.count, [mdlMesh.name UTF8String], mdlMesh.vertexCount
    );

    NSArray<MTKSubmesh*>* submeshes = mesh.submeshes;

    int j = 0;
    for (MTKSubmesh* mtkSubmesh : submeshes) {
      MDLSubmesh* mdlSubmesh = mdlMesh.submeshes[j];
      MDLMaterial* mdlMaterial = [mdlSubmesh material];

      Renderer::Material material = [TextureRepository readMaterial:device material:mdlMaterial];
      material.useColor = (!useTexture);
      material.useLight = useLight;
			
			std::vector<MTL::Texture*> textures;
      if (useTexture) texture = [TextureRepository read:device material:mdlSubmesh.material];
			textures.emplace_back(texture);
      expMesh->add(new Explorer::Submesh(
          material,
          textures,
          (__bridge MTL::PrimitiveType)mtkSubmesh.primitiveType,
          mtkSubmesh.indexCount,
          (__bridge MTL::IndexType)mtkSubmesh.indexType,
          (__bridge MTL::Buffer*)mtkSubmesh.indexBuffer.buffer,
          mtkSubmesh.indexBuffer.offset
      ));
      j += 1;
    }
    expMeshes.push_back(expMesh);
    i += 1;
  }
  Explorer::Model* model =
      new Explorer::Model(expMeshes, path.erase(0, path.find_last_of("/") + 1), tvCount);
  DEBUG("MeshRepository -> " + model->toString());
  return model;
}

Explorer::Submesh*
buildSubmesh(MTL::Device* device, MTKSubmesh* mtkSubmesh, MDLSubmesh* mdlSubmesh) {
  Renderer::Material material = [TextureRepository readMaterial:device
                                                       material:mdlSubmesh.material];
  std::vector<MTL::Texture*> textures;

  textures.emplace_back([TextureRepository read2:device
                                        material:mdlSubmesh.material
                                        semantic:MDLMaterialSemanticBaseColor]);
  
  textures.emplace_back([TextureRepository read2:device
                                   material:mdlSubmesh.material
                                   semantic:MDLMaterialSemanticMetallic]);
  textures.emplace_back([TextureRepository read2:device
                                   material:mdlSubmesh.material
                                   semantic:MDLMaterialSemanticRoughness]);
  textures.emplace_back([TextureRepository read2:device
                                   material:mdlSubmesh.material
                                   semantic:MDLMaterialSemanticTangentSpaceNormal]);
  textures.emplace_back([TextureRepository read2:device
                                   material:mdlSubmesh.material
                                   semantic:MDLMaterialSemanticAmbientOcclusion]);
    
  Explorer::Submesh* submesh = new Explorer::Submesh(
      material,
      textures,
      (__bridge MTL::PrimitiveType)mtkSubmesh.primitiveType,
      mtkSubmesh.indexCount,
      (__bridge MTL::IndexType)mtkSubmesh.indexType,
      (__bridge MTL::Buffer*)mtkSubmesh.indexBuffer.buffer,
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

Explorer::Mesh*
buildMesh(MTL::Device* device, MDLMesh* mdlMesh, MTL::VertexDescriptor* vertexDescriptor) {
  id<MTLDevice> objcppDevice = (__bridge id<MTLDevice>)device;

  DEBUG("Adding normals to mesh...");
  [mdlMesh addNormalsWithAttributeNamed:MDLVertexAttributeNormal creaseThreshold:0.98];

  DEBUG("Building mdl vertex descriptor...");
  mdlMesh.vertexDescriptor = buildMDLVertexDescriptor(device, vertexDescriptor);

  DEBUG("Initializing mtk mesh...");
  NSError* err = nil;
  MTKMesh* mtkMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:objcppDevice error:&err];
  if (err) Explorer::printError((__bridge NS::Error*)err);
  assert(mtkMesh.submeshes.count == mtkMesh.submeshes.count);

  DEBUG("Number of vertex buffers detected: " + std::to_string(mtkMesh.vertexBuffers.count));
  DEBUG("Initializing explorer mesh...");

  std::vector<MTL::Buffer*> buffers;
  std::vector<int> offsets;
  for (int i = 0; i < mtkMesh.vertexBuffers.count; i++) {
    MTL::Buffer* buffer = (__bridge MTL::Buffer*)[mtkMesh.vertexBuffers objectAtIndex:i].buffer;
    int offset = [mtkMesh.vertexBuffers objectAtIndex:i].offset;

    buffers.emplace_back(buffer);
    offsets.emplace_back(offset);
  }

  Explorer::Mesh* mesh = new Explorer::Mesh(
      buffers,
      offsets,
      mtkMesh.vertexBuffers.count,
      [[mdlMesh name] UTF8String],
      mdlMesh.vertexCount
  );

  std::stringstream ss;
  ss << "Found " << mtkMesh.submeshes.count << " submeshes.";
  DEBUG(ss.str());

  for (int i = 0; i < mtkMesh.submeshes.count; i++) {
    Explorer::Submesh* submesh = buildSubmesh(device, mtkMesh.submeshes[i], mdlMesh.submeshes[i]);
    DEBUG("Adding mesh to array...");

    mesh->add(submesh);
  }
  DEBUG("Returning mesh...");
  return mesh;
}

std::vector<Explorer::Mesh*>
buildMeshes(MTL::Device* device, MDLObject* object, MTL::VertexDescriptor* vertexDescriptor) {
  std::vector<Explorer::Mesh*> meshes;
  if ([object isKindOfClass:[MDLMesh class]]) {
    DEBUG("Found MDLMesh class. Building mesh...");
    Explorer::Mesh* mesh = buildMesh(device, (MDLMesh*)object, vertexDescriptor);
    meshes.emplace_back(mesh);
  }

  for (MDLObject* child in object.children) {
    DEBUG("Found MDLObject class. Recursing...");
    std::vector<Explorer::Mesh*> meshes = buildMeshes(device, child, vertexDescriptor);
    meshes.insert(meshes.end(), meshes.begin(), meshes.end());
  }
  return meshes;
}

/**
Explorer::Model*
Repository::Meshes::read2(MTL::Device* device, std::string path, bool
useTexture, bool useLight) { return nullptr;
}
**/

Explorer::Model* Repository::Meshes::read2(
    MTL::Device* device,
    MTL::VertexDescriptor* vertexDescriptor,
    std::string path,
    bool useTexture,
    bool useLight
) {
  DEBUG("Reading mesh...");

  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path + ".obj");
  MTKMeshBufferAllocator* bufferAllocator =
      [[MTKMeshBufferAllocator alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  DEBUG("Reading asset...");
  MDLAsset* mdlAsset = [[MDLAsset alloc] initWithURL:url
                                    vertexDescriptor:nil
                                     bufferAllocator:bufferAllocator];

  MTKTextureLoader* textureLoader =
      [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  std::vector<Explorer::Mesh*> assetMeshes;
  for (MDLObject* mdlObject : mdlAsset) {
    DEBUG("One object to meshes...");
    std::vector<Explorer::Mesh*> objMeshes = buildMeshes(device, mdlObject, vertexDescriptor);

    for (int i = 0; i < objMeshes.size(); i++) {
      DEBUG("Adding mesh to vector ...");
      std::stringstream ss;
      ss << objMeshes.size();
      DEBUG(ss.str());
      assetMeshes.emplace_back(objMeshes[i]);
    }
  }
  DEBUG("Initializing model ...");
  Explorer::Model* model = new Explorer::Model(assetMeshes, "model", -1);
  DEBUG("MeshRepository -> " + model->toString());
  return model;
}

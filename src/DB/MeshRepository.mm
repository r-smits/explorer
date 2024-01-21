#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <MetalKit/MetalKit.h>
#include <Model/MeshFactory.h>
#include <ModelIO/ModelIO.h>
#include <Renderer/Descriptor.h>

Explorer::Model* Repository::Meshes::read(
    MTL::Device* device, std::string path = "", bool useTexture, bool useLight
) {
  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path + ".obj");
  MTL::VertexDescriptor* mtlVertexDescriptor =
      Renderer::Descriptor::vertex(device, Renderer::Layouts::vertex);
  MDLVertexDescriptor* mdlVertexDescriptor =
      MTKModelIOVertexDescriptorFromMetal((__bridge MTLVertexDescriptor*)mtlVertexDescriptor);

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

  std::vector<Explorer::Mesh> expMeshes;
  MTL::Texture* texture = Repository::Textures::read(device, path);

  int tvCount = 0;

  int i = 0;
  for (MTKMesh* mesh : meshes) {

    MDLMesh* mdlMesh = mdlMeshes[i];
    MTKMeshBuffer* meshBuffer = [mesh.vertexBuffers objectAtIndex:0];
    // MTL::Buffer* vertexBuffer = (__bridge MTL::Buffer*)[mesh.vertexBuffers
    // objectAtIndex:0].buffer;

    int vertexCount = mdlMesh.vertexCount;
    tvCount += vertexCount;

    MTL::Buffer* mtkMeshVertexBuffer = (__bridge MTL::Buffer*)mesh.vertexBuffers.firstObject.buffer;
    int mtkMeshVertexBufferOffset = mesh.vertexBuffers.firstObject.offset;

    Explorer::Mesh expMesh = Explorer::Mesh(
        mtkMeshVertexBuffer,
        mtkMeshVertexBufferOffset,
        [mdlMesh.name UTF8String],
        mdlMesh.vertexCount
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
      if (useTexture)
        textures.emplace_back([TextureRepository read:device material:mdlSubmesh.material]);
      expMesh.add(Explorer::Submesh(
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
  Explorer::Model* model = new Explorer::Model(expMeshes, "", tvCount);
  DEBUG("MeshRepository -> " + model->toString());
  return model;
}

Explorer::Submesh
buildSubmesh(MTL::Device* device, MTKSubmesh* mtkSubmesh, MDLSubmesh* mdlSubmesh) {
  Renderer::Material material = [TextureRepository readMaterial:device
                                                       material:mdlSubmesh.material];
  std::vector<MTL::Texture*> textures;

  /**
textures.emplace_back([TextureRepository read:device
                                 material:mdlSubmesh.material
                                 semantic:MDLMaterialSemanticBaseColor]);

  textures.emplace_back([TextureRepository read:device
                                 material:mdlSubmesh.material
                                 semantic:MDLMaterialSemanticMetallic]);
textures.emplace_back([TextureRepository read:device
                                 material:mdlSubmesh.material
                                 semantic:MDLMaterialSemanticRoughness]);
textures.emplace_back([TextureRepository read:device
                                 material:mdlSubmesh.material
                                 semantic:MDLMaterialSemanticTangentSpaceNormal]);
textures.emplace_back([TextureRepository read:device
                                 material:mdlSubmesh.material
                                 semantic:MDLMaterialSemanticAmbientOcclusion]);
  **/
  Explorer::Submesh submesh = Explorer::Submesh(
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
buildMDLVertexDescriptor(MTL::Device* device, Renderer::BufferLayouts layouts) {

  MTL::VertexDescriptor* vertexDescriptor = Renderer::Descriptor::vertex(device, layouts);
  MDLVertexDescriptor* mdlVertexDescriptor =
      MTKModelIOVertexDescriptorFromMetal((__bridge MTLVertexDescriptor*)vertexDescriptor);
  [[[mdlVertexDescriptor attributes] objectAtIndex:0] setName:MDLVertexAttributePosition];
  [[[mdlVertexDescriptor attributes] objectAtIndex:1] setName:MDLVertexAttributeColor];
  [[[mdlVertexDescriptor attributes] objectAtIndex:2] setName:MDLVertexAttributeTextureCoordinate];
  [[[mdlVertexDescriptor attributes] objectAtIndex:3] setName:MDLVertexAttributeNormal];
  return mdlVertexDescriptor;
}

Explorer::Mesh buildMesh(MTL::Device* device, MDLMesh* mdlMesh) {
  id<MTLDevice> objcppDevice = (__bridge id<MTLDevice>)device;

  DEBUG("Adding normals to mesh...");
  [mdlMesh addNormalsWithAttributeNamed:MDLVertexAttributeNormal creaseThreshold:0.98];

  DEBUG("Building mdl vertex descriptor...");
  mdlMesh.vertexDescriptor = buildMDLVertexDescriptor(device, Renderer::Layouts::vertex);

  DEBUG("Initializing mtk mesh...");
  NSError* err = nil;
  MTKMesh* mtkMesh = [[MTKMesh alloc] initWithMesh:mdlMesh device:objcppDevice error:&err];
  if (err) Explorer::printError((__bridge NS::Error*)err);
  assert(mtkMesh.submeshes.count == mtkMesh.submeshes.count);

  DEBUG("Initializing explorer mesh...");
  Explorer::Mesh mesh = Explorer::Mesh(
      (__bridge MTL::Buffer*)mtkMesh.vertexBuffers.firstObject.buffer,
      mtkMesh.vertexBuffers.firstObject.offset,
      [[mdlMesh name] UTF8String],
      mdlMesh.vertexCount
  );

  std::stringstream ss;
  ss << "Found " << mtkMesh.submeshes.count << " submeshes.";
  DEBUG(ss.str());

  for (int i = 0; i < mtkMesh.submeshes.count; i++) {
    Explorer::Submesh submesh = buildSubmesh(device, mtkMesh.submeshes[i], mdlMesh.submeshes[i]);
    DEBUG("Adding mesh to array...");

    mesh.add(submesh);
  }
  DEBUG("Returning mesh...");
  return mesh;
}

std::vector<Explorer::Mesh> buildMeshes(MTL::Device* device, MDLObject* object) {
  std::vector<Explorer::Mesh> meshes;
  if ([object isKindOfClass:[MDLMesh class]]) {
    DEBUG("Found MDLMesh class. Building mesh...");
    Explorer::Mesh mesh = buildMesh(device, (MDLMesh*)object);
    meshes.emplace_back(mesh);
  }

  // No recursion for now. Either it's a mesh or not.
  /**
for (MDLObject* child in object.children) {
          DEBUG("Found MDLObject class. Recursing...");
std::vector<Explorer::Mesh*> meshes = buildMeshes(device, child, textureLoader);
totalMeshes.insert(meshes.end(), meshes.begin(), meshes.end());
}
  **/
  return meshes;
}

Explorer::Model* Repository::Meshes::read2(
    MTL::Device* device, const std::string& path = "", bool useTexture, bool useLight
) {
  DEBUG("Reading mesh...");
  id<MTLDevice> objcppDevice = (__bridge id<MTLDevice>)device;
  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path + ".obj");
  MTKMeshBufferAllocator* bufferAllocator =
      [[MTKMeshBufferAllocator alloc] initWithDevice:objcppDevice];

  DEBUG("Reading asset...");
  MDLAsset* mdlAsset = [[MDLAsset alloc] initWithURL:url
                                    vertexDescriptor:nil
                                     bufferAllocator:bufferAllocator];

  MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:objcppDevice];

  std::vector<Explorer::Mesh> allMeshes;
  for (MDLObject* mdlObject : mdlAsset) {
    DEBUG("One object to meshes...");
    std::vector<Explorer::Mesh> meshes = buildMeshes(device, mdlObject);

    for (int i = 0; i < meshes.size(); i++) {
      DEBUG("Adding mesh to vector ...");
      std::stringstream ss;

      ss << meshes.size();

      DEBUG(ss.str());
      allMeshes.emplace_back(meshes[i]);
    }
    DEBUG("Inserting meshes into vector...");
    // allMeshes.insert(meshes.end(), meshes.begin(), meshes.end());
    DEBUG("Done inserting meshes into the vector...");
  }
  DEBUG("Initializing model ...");
  Explorer::Model* model = new Explorer::Model(allMeshes);
  DEBUG("MeshRepository -> " + model->toString());
  return model;
}

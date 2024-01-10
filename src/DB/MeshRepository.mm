#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <MetalKit/MetalKit.h>
#include <Model/MeshFactory.h>
#include <ModelIO/ModelIO.h>
#include <Renderer/Descriptor.h>

Explorer::Model*
Repository::Meshes::read(MTL::Device* device, std::string path = "", bool useTexture) {

  DEBUG("Loading mesh: " + path);

  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path + ".obj");
  MTL::VertexDescriptor* mtlVertexDescriptor =
      Renderer::Descriptor::vertex(device, &Renderer::Layouts::vertex);
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

  Renderer::Material placeholderMaterial = {
      {1.0f, 1.0f, 1.0f, 1.0f}, // color; white
      {0.1f, 0.1f, 0.1f}, // ambient intensity -> low
      {1.0f, 1.0f, 1.0f}, // diffuse intensity -> high
      {0.0f, 0.0f, 0.0f}, // specular intensity -> off
      (!useTexture), // useTexture: false -> useColor: true
  };

  std::vector<Explorer::Mesh*> expMeshes;

  MTL::Texture* texture = Repository::Textures::read(device, path);

  int i = 0;
  for (MTKMesh* mesh : meshes) {

    MDLMesh* mdlMesh = mdlMeshes[i];

    MTKMeshBuffer* meshBuffer = [mesh.vertexBuffers objectAtIndex:0];

    MTL::Buffer* vertexBuffer = (__bridge MTL::Buffer*)[mesh.vertexBuffers objectAtIndex:0].buffer;

    int vertexCount = mdlMesh.vertexCount;

    Explorer::Mesh* expMesh = new Explorer::Mesh(vertexBuffer);

    NSArray<MTKSubmesh*>* submeshes = mesh.submeshes;

    int j = 0;
    for (MTKSubmesh* mtkSubmesh : submeshes) {
      MDLSubmesh* mdlSubmesh = mdlMesh.submeshes[j];

      if (useTexture) texture = [TextureRepository read:device material:mdlSubmesh.material];
      expMesh->add(new Explorer::Submesh(
          placeholderMaterial,
          texture,
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
  return new Explorer::Model(expMeshes);
}

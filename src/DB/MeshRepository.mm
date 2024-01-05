#include <DB/Repository.h>
#include <MetalKit/MetalKit.h>
#include <Model/MeshFactory.h>
#include <Renderer/Descriptor.h>

std::vector<Explorer::Mesh*> Repository::Meshes::read(MTL::Device* device, std::string path) {
  NSURL* url = (__bridge NSURL*)Explorer::nsUrl(path);
  MTL::VertexDescriptor* descriptor =
      Renderer::Descriptor::vertex(device, &Renderer::Layouts::vertex);
  MDLVertexDescriptor* modelDescriptor =
      MTKModelIOVertexDescriptorFromMetal((__bridge MTLVertexDescriptor*)descriptor);

  MDLVertexAttribute* position = [[modelDescriptor attributes] objectAtIndex:0];
  [position setName:MDLVertexAttributePosition];
  [[modelDescriptor attributes] setObject:position atIndexedSubscript:0];

  MDLVertexAttribute* color = [[modelDescriptor attributes] objectAtIndex:1];
  [color setName:MDLVertexAttributePosition];
  [[modelDescriptor attributes] setObject:color atIndexedSubscript:1];

  MDLVertexAttribute* texture = [[modelDescriptor attributes] objectAtIndex:2];
  [texture setName:MDLVertexAttributePosition];
  [[modelDescriptor attributes] setObject:texture atIndexedSubscript:2];

  MTKMeshBufferAllocator* bufferAllocator =
      [[MTKMeshBufferAllocator alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  MDLAsset* asset = [[MDLAsset alloc] initWithURL:url
                                 vertexDescriptor:modelDescriptor
                                  bufferAllocator:bufferAllocator];

  // NSError* err = nil;
  NSArray<MTKMesh*>* meshes = [MTKMesh newMeshesFromAsset:asset
                                                   device:(__bridge id<MTLDevice>)device
                                             sourceMeshes:nil
                                                    error:nil];

  std::vector<Explorer::Mesh*> expMeshes;

  for (MTKMesh* mesh : meshes) {
    MTKMeshBuffer* meshBuffer = [mesh.vertexBuffers objectAtIndex:0];

    MTL::Buffer* vertexBuffer = (__bridge MTL::Buffer*)[mesh.vertexBuffers objectAtIndex:0].buffer;

    Explorer::Mesh* expMesh =
        new Explorer::Mesh(vertexBuffer, Repository::Textures::read(device, path));

    NSArray<MTKSubmesh*>* submeshes = mesh.submeshes;
    for (MTKSubmesh* mtkSubmesh : submeshes) {
      expMesh->add(new Explorer::Submesh(
          (__bridge MTL::PrimitiveType)mtkSubmesh.primitiveType,
          mtkSubmesh.indexCount,
          (__bridge MTL::IndexType)mtkSubmesh.indexType,
          (__bridge MTL::Buffer*)mtkSubmesh.indexBuffer.buffer,
          mtkSubmesh.indexBuffer.offset
      ));
    }

    expMeshes.push_back(expMesh);
  }
  return expMeshes;
}

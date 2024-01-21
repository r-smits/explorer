#include <DB/Repository.hpp>
#include <Renderer/Descriptor.h>

MTL::VertexDescriptor*
Renderer::Descriptor::vertex(MTL::Device* device, const BufferLayouts& layouts) {
  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  int i, j = 0;
  for (BufferLayout layout : layouts.getLayouts()) {
    int stride = 0;
    for (BufferElement element : layout.getElements()) {
      MTL::VertexAttributeDescriptor* positionDescriptor =
          vertexDescriptor->attributes()->object(i);
      positionDescriptor->setFormat((MTL::VertexFormat)element.type);
      positionDescriptor->setOffset(stride);
      positionDescriptor->setBufferIndex(j);
      stride += element.size;
      i += 1;
    }
    MTL::VertexBufferLayoutDescriptor* layoutDescriptor = vertexDescriptor->layouts()->object(j);
    layoutDescriptor->setStride(stride);
    j += 1;
  }
  return vertexDescriptor;
}

Renderer::PDV Renderer::Descriptor::primitives(Explorer::Model* model) {
  PDV result;
  for (auto mesh : model->meshes)
    result.emplace_back(Descriptor::primitive(mesh));
  return result;
}

MTL::RenderPipelineDescriptor*
Renderer::Descriptor::render(MTL::Device* device, const std::string& path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  // Add flags to state adding binary functions is supported
  descriptor->setSupportAddingVertexBinaryFunctions(true);
  descriptor->setSupportAddingFragmentBinaryFunctions(true);

  // Add functions to the pipeline descriptor
  descriptor->setVertexFunction(library->newFunction(Explorer::nsString("vertexMainGeneral")));
  descriptor->setFragmentFunction(library->newFunction(Explorer::nsString("fragmentMainGeneral")));

  descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB
  );

  descriptor->setVertexDescriptor(Renderer::Descriptor::vertex(device, Renderer::Layouts::vertex));
  library->release();
  return descriptor;
}

MTL::ComputePipelineDescriptor*
Renderer::Descriptor::compute(MTL::Device* device, const std::string& path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::ComputePipelineDescriptor* descriptor = MTL::ComputePipelineDescriptor::alloc()->init();
  descriptor->setSupportAddingBinaryFunctions(true);
  descriptor->setComputeFunction(library->newFunction(Explorer::nsString("computeKernel")));
  library->release();
  return descriptor;
}

MTL::InstanceAccelerationStructureDescriptor* Renderer::Descriptor::instance(
    MTL::Device* device, NS::Array* structures, const int& instanceCount
) {
  MTL::InstanceAccelerationStructureDescriptor* descriptor =
      MTL::InstanceAccelerationStructureDescriptor::descriptor();

  descriptor->setInstancedAccelerationStructures(structures);
  descriptor->setInstanceCount(instanceCount);

  MTL::Buffer* instanceBuffer = device->newBuffer(
      sizeof(MTL::AccelerationStructureInstanceDescriptor) * instanceCount,
      MTL::ResourceStorageModeShared
  );

  MTL::AccelerationStructureInstanceDescriptor* descriptors =
      (MTL::AccelerationStructureInstanceDescriptor*)instanceBuffer->contents();

  for (int i = 0; i < instanceCount; i++) {
    // Need to figure out what this references;
    descriptors[i].accelerationStructureIndex;
    descriptors[i].intersectionFunctionTableOffset = 0;
    descriptors[i].mask = 0xFF;
    descriptors[i].options = MTL::AccelerationStructureInstanceOptionNone;
  }

  return descriptor;
}

/**
MTLInstanceAccelerationStructureDescriptor *instanceAccelStructureDesc =
      [MTLInstanceAccelerationStructureDescriptor descriptor];
  instanceAccelStructureDesc.instancedAccelerationStructures =
      _primitiveAccelerationStructures;

  instanceAccelStructureDesc.instanceCount = kMaxInstances;

  // Load instance data (two fire trucks + one sphere + floor):

  id<MTLBuffer> instanceDescriptorBuffer = [_device
      newBufferWithLength:sizeof(MTLAccelerationStructureInstanceDescriptor) *
                          kMaxInstances
                  options:MTLResourceStorageModeShared];


  MTLAccelerationStructureInstanceDescriptor *instanceDescriptors =
      (MTLAccelerationStructureInstanceDescriptor *)
          instanceDescriptorBuffer.contents;
  for (NSUInteger i = 0; i < kMaxInstances; ++i) {
    instanceDescriptors[i].accelerationStructureIndex =
        _modelInstances[i].meshIndex;
    instanceDescriptors[i].intersectionFunctionTableOffset = 0;
    instanceDescriptors[i].mask = 0xFF;
    instanceDescriptors[i].options = MTLAccelerationStructureInstanceOptionNone;

    AAPLInstanceTransform *transforms =
        (AAPLInstanceTransform *)(((uint8_t *)
                                       _instanceTransformBuffer.contents) +
                                  i * kAlignedInstanceTransformsStructSize);
    instanceDescriptors[i].transformationMatrix =
        matrix4x4_drop_last_row(transforms->modelViewMatrix);
  }
  instanceAccelStructureDesc.instanceDescriptorBuffer =
      instanceDescriptorBuffer;

  id<MTLCommandBuffer> cmd = [_commandQueue commandBuffer];
  [cmd encodeWaitForEvent:_accelerationStructureBuildEvent
                    value:kPrimitiveAccelerationStructureBuild];
  _instanceAccelerationStructure =
      [self allocateAndBuildAccelerationStructureWithDescriptor:
                instanceAccelStructureDesc
                                                  commandBuffer:cmd];
  [cmd encodeSignalEvent:_accelerationStructureBuildEvent
                   value:kInstanceAccelerationStructureBuild];
  [cmd commit];
**/

// For every mesh :: 1 PrimitiveAccelerationStructure
// For every submesh :: 1 Geometry
MTL::PrimitiveAccelerationStructureDescriptor*
Renderer::Descriptor::primitive(const Explorer::Mesh& mesh) {
  std::vector<MTL::AccelerationStructureTriangleGeometryDescriptor*> dGeometries;

  std::vector<Explorer::Submesh> submeshes = mesh.submeshes();

  for (int i = 0; i < mesh.count; i++) {
    MTL::AccelerationStructureTriangleGeometryDescriptor* dGeometry =
        MTL::AccelerationStructureTriangleGeometryDescriptor::descriptor();
    Explorer::Submesh submesh = submeshes[i];

    dGeometry->setVertexBuffer(mesh.vertexBuffer);
    dGeometry->setVertexBufferOffset(mesh.vertexBufferOffset);

    // Q: Should I be using packed floats here?
    // For now: 4float x 4 (due to padding) == 64 bytes
    // Future: use layoutDescriptor instead
    dGeometry->setVertexStride(64);
    dGeometry->setIndexBuffer(submesh.indexBuffer);
    dGeometry->setIndexType(submesh.indexType);
    dGeometry->setIndexBufferOffset(submesh.offset);
    dGeometry->setTriangleCount(submesh.indexCount / 3);

    dGeometries.emplace_back(dGeometry);
  }

  MTL::PrimitiveAccelerationStructureDescriptor* dPrimitive =
      MTL::PrimitiveAccelerationStructureDescriptor::descriptor();
  NS::Array* aGeometries =
      NS::Array::array((NS::Object* const*)&dGeometries[0], dGeometries.size());
  dPrimitive->setGeometryDescriptors(aGeometries);
  return dPrimitive;
}

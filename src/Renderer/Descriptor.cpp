#include <DB/Repository.hpp>
#include <Renderer/Descriptor.h>

MTL::VertexDescriptor*
Renderer::Descriptor::vertex(MTL::Device* device, const BufferLayouts& layouts) {
  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  int elementIndex = 0;
  std::vector<BufferLayout> bufferLayouts = layouts.getLayouts();
  for (int layoutIndex = 0; layoutIndex < layouts.count; ++layoutIndex) {

    BufferLayout layout = bufferLayouts[layoutIndex];
    int stride = 0;

    for (BufferElement element : layout.getElements()) {
      MTL::VertexAttributeDescriptor* positionDescriptor =
          vertexDescriptor->attributes()->object(elementIndex);
      positionDescriptor->setFormat((MTL::VertexFormat)element.type);
      positionDescriptor->setOffset(stride);
      positionDescriptor->setBufferIndex(layoutIndex);
      stride += element.size;
      elementIndex += 1;
    }
    MTL::VertexBufferLayoutDescriptor* layoutDescriptor =
        vertexDescriptor->layouts()->object(layoutIndex);
    layoutDescriptor->setStride(stride);
    layoutDescriptor->setStepRate(1);
    layoutDescriptor->setStepFunction(MTL::VertexStepFunctionPerVertex);
  }

  return vertexDescriptor;
}

MTL::RenderPipelineDescriptor* Renderer::Descriptor::render(
    MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, const std::string& path
) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  // Add flags to state adding binary functions is supported
  descriptor->setSupportAddingVertexBinaryFunctions(true);
  descriptor->setSupportAddingFragmentBinaryFunctions(true);

  // Add functions to the pipeline descriptor
  descriptor->setVertexFunction(library->newFunction(EXP::nsString("vertexMainGeneral")));
  descriptor->setFragmentFunction(library->newFunction(EXP::nsString("fragmentMainGeneral")));

  descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB
  );

  descriptor->setVertexDescriptor(vertexDescriptor);
  library->release();
  return descriptor;
}

MTL::ComputePipelineDescriptor*
Renderer::Descriptor::compute(MTL::Device* device, const std::string& path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::ComputePipelineDescriptor* descriptor = MTL::ComputePipelineDescriptor::alloc()->init();
  descriptor->setSupportAddingBinaryFunctions(true);
  descriptor->setComputeFunction(library->newFunction(EXP::nsString("computeKernel")));
  library->release();
  return descriptor;
}

std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> Renderer::Descriptor::primitives(
    const std::vector<EXP::Model*>& scene, const int& vStride, const int& pStride
) {
  std::vector<EXP::Mesh*> meshes;
  for (EXP::Model* model : scene) {
    for (EXP::Mesh* mesh : model->meshes)
      meshes.emplace_back(mesh);
  }
  std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> descriptors;
  for (EXP::Mesh* mesh : meshes)
    descriptors.emplace_back(Descriptor::primitive(mesh, vStride, pStride));
  return descriptors;
}

// For every mesh :: 1 PrimitiveAccelerationStructure, 1 instance
// For every submesh :: 1 Geometry
MTL::PrimitiveAccelerationStructureDescriptor*
Renderer::Descriptor::primitive(EXP::Mesh* mesh, const int& vStride, const int& pStride) {

  std::vector<MTL::AccelerationStructureTriangleGeometryDescriptor*> geometryDescriptors;
  std::vector<EXP::MDL::Submesh*> submeshes = mesh->submeshes();

  for (int i = 0; i < mesh->count; i++) {
    MTL::AccelerationStructureTriangleGeometryDescriptor* geometryDescriptor =
        MTL::AccelerationStructureTriangleGeometryDescriptor::descriptor();
    EXP::MDL::Submesh* submesh = submeshes[i];

    // A: Using a packed float, set to 3float in buffer
    // Using non-interleaved vertex buffer
    geometryDescriptor->setVertexBuffer(mesh->buffers[0]);
    geometryDescriptor->setVertexBufferOffset(mesh->offsets[0]);
    geometryDescriptor->setVertexStride(vStride);

    // You should be able to set the primitive data buffer here
    geometryDescriptor->setPrimitiveDataBuffer(submesh->primitiveBuffer);
    geometryDescriptor->setPrimitiveDataBufferOffset(submesh->offset);
    geometryDescriptor->setPrimitiveDataElementSize(sizeof(Renderer::PrimitiveAttributes));
    geometryDescriptor->setPrimitiveDataStride(sizeof(Renderer::PrimitiveAttributes));

    geometryDescriptor->setIndexBuffer(submesh->indexBuffer);
    geometryDescriptor->setIndexType(submesh->indexType);
    geometryDescriptor->setIndexBufferOffset(submesh->offset);
    geometryDescriptor->setTriangleCount(submesh->indexCount / 3);
    geometryDescriptors.emplace_back(geometryDescriptor);
  }

  MTL::PrimitiveAccelerationStructureDescriptor* dPrimitive =
      MTL::PrimitiveAccelerationStructureDescriptor::descriptor();
  NS::Array* aGeometries =
      NS::Array::array((NS::Object* const*)&geometryDescriptors[0], mesh->count);
  dPrimitive->setGeometryDescriptors(aGeometries);
  return dPrimitive;
}

MTL::InstanceAccelerationStructureDescriptor* Renderer::Descriptor::instance(
    MTL::Device* device,
    const std::vector<MTL::AccelerationStructure*>& primitiveStructures,
    const std::vector<EXP::Model*>& scene
) {
  int count = 0;
  for (EXP::Model* model : scene)
    count += model->meshCount;
  EXP::Mesh* meshes[count];

  int currentCount = 0;
  for (EXP::Model* model : scene) {
    for (EXP::Mesh* mesh : model->meshes) {
      meshes[currentCount] = mesh;
      currentCount += 1;
    }
  }

  MTL::InstanceAccelerationStructureDescriptor* descriptor =
      MTL::InstanceAccelerationStructureDescriptor::descriptor();
  NS::Array* primitiveStructuresArray =
      NS::Array::array((NS::Object* const*)&primitiveStructures[0], primitiveStructures.size());
  descriptor->setInstancedAccelerationStructures(primitiveStructuresArray);
  descriptor->setInstanceCount(count);

  MTL::Buffer* instanceDescriptorBuffer = device->newBuffer(
      sizeof(MTL::AccelerationStructureInstanceDescriptor) * count, MTL::ResourceStorageModeShared
  );

  MTL::AccelerationStructureInstanceDescriptor* instanceDescriptors =
      (MTL::AccelerationStructureInstanceDescriptor*)instanceDescriptorBuffer->contents();

  for (int i = 0; i < count; i++) {
    instanceDescriptors[i].accelerationStructureIndex = i;
    instanceDescriptors[i].intersectionFunctionTableOffset = 0;
    instanceDescriptors[i].mask = 0xFF;
    instanceDescriptors[i].options = MTL::AccelerationStructureInstanceOptionNone;
    MTL::PackedFloat4x3 modelViewTransform = EXP::MATH::pack(meshes[i]->f4x4()->get());
    instanceDescriptors[i].transformationMatrix = modelViewTransform;
  }

  descriptor->setInstanceDescriptorBuffer(instanceDescriptorBuffer);
  return descriptor;
}

MTL::InstanceAccelerationStructureDescriptor* Renderer::Descriptor::updateTransformationMatrix(
    const std::vector<EXP::Model*>& scene,
    MTL::InstanceAccelerationStructureDescriptor* descriptor
) {
  int count = 0;
  for (EXP::Model* model : scene)
    count += model->meshCount;

  EXP::Mesh* meshes[count];
  int currentCount = 0;
  for (EXP::Model* model : scene) {
    for (EXP::Mesh* mesh : model->meshes) {
      meshes[currentCount] = mesh;
      currentCount += 1;
    }
  }

  MTL::Buffer* descriptorBuffer = descriptor->instanceDescriptorBuffer();
  auto* descriptors = (MTL::AccelerationStructureInstanceDescriptor*)descriptorBuffer->contents();
  for (int i = 0; i < count; i++) {
    descriptors[i].transformationMatrix = EXP::MATH::pack(meshes[i]->f4x4()->get());
  }
  descriptor->setInstanceDescriptorBuffer(descriptorBuffer);
  return descriptor;
}

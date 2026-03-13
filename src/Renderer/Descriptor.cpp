#include <DB/Repository.hpp>
#include <Renderer/Descriptor.h>


MTL::VertexDescriptor* Renderer::Descriptor::vertex(
  MTL::Device* device, 
  const BufferLayouts& layouts
) {
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

MTL::ComputePipelineDescriptor* Renderer::Descriptor::compute(MTL::Device* device, const std::string& path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::ComputePipelineDescriptor* descriptor = MTL::ComputePipelineDescriptor::alloc()->init();
  descriptor->setSupportAddingBinaryFunctions(true);
  descriptor->setComputeFunction(library->newFunction(EXP::nsString("computeKernel")));
  library->release();
  return descriptor;
}

std::vector<prim_acc_desc*> Renderer::Descriptor::primitives(
  mesh_array meshes, 
  const int& vStride, 
  const int& pStride
) {
  std::vector<prim_acc_desc*> descriptors;  
  for (auto mesh : meshes) descriptors.emplace_back(Descriptor::primitive(mesh, vStride, pStride));
  return descriptors;
}


// For every mesh :: 1 PrimitiveAccelerationStructure, 1 instance
// For every submesh :: 1 Geometry
prim_acc_desc* Renderer::Descriptor::primitive(
  EXP::MDL::Mesh* mesh, 
  const int& vStride, 
  const int& pStride
) {
  std::vector<geom_desc*> geometryDescriptors;
  std::vector<EXP::MDL::Submesh*> submeshes = mesh->getSubmeshes();
  prim_acc_desc* dPrimitive = prim_acc_desc::descriptor();
 
  for (int i = 0; i < mesh->count; i++) {
    geom_desc* geometryDescriptor = geom_desc::descriptor();
    EXP::MDL::Submesh* submesh = submeshes[i];
    // A: Using a packed float, set to 3float in buffer
    // Using non-interleaved vertex buffer
    geometryDescriptor->setVertexBuffer(mesh->buffers[0]);
    geometryDescriptor->setVertexBufferOffset(mesh->offsets[0]);
    geometryDescriptor->setVertexStride(vStride);

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
  dPrimitive->setGeometryDescriptors(
    NS::Array::array(reinterpret_cast<NS::Object* const*>(&geometryDescriptors[0]), mesh->count)
  );
  return dPrimitive;
}


inst_acc_desc* Renderer::Descriptor::instance(
    MTL::Device* device,
    acc_array primitiveStructures,
    mesh_array meshes
) {
  inst_acc_desc* descriptor = inst_acc_desc::descriptor();  
  descriptor->setInstancedAccelerationStructures(
    NS::Array::array(reinterpret_cast<NS::Object* const*>(&primitiveStructures[0]), primitiveStructures.size())
  );
  descriptor->setInstanceCount(meshes.size());
  size_t inst_desc_size = sizeof(inst_desc) * meshes.size();
  MTL::Buffer* instanceDescriptorBuffer = device->newBuffer(inst_desc_size, MTL::ResourceStorageModeShared);
  inst_desc* instanceDescriptors = static_cast<inst_desc*>(instanceDescriptorBuffer->contents());
  for (unsigned int i = 0; i < meshes.size(); i+=1) {
    instanceDescriptors[i] = {
      .accelerationStructureIndex = i,
      .intersectionFunctionTableOffset = 0,
      .mask = 0xFF,
      .options = MTL::AccelerationStructureInstanceOptionNone,
      .transformationMatrix = EXP::MATH::pack(meshes[i]->f4x4()->get())
    };
  }
  descriptor->setInstanceDescriptorBuffer(instanceDescriptorBuffer);
  return descriptor;
}


// NOTE TO SELF: INST_ACC_DESC contains ACC_INST_DEC
// SO: INST_ACC_DESC > ACC_INST_DEC
inst_acc_desc* Renderer::Descriptor::updateTransformationMatrix(
  mesh_array meshes, 
  inst_acc_desc* descriptor
) {
  MTL::Buffer* descriptorBuffer = descriptor->instanceDescriptorBuffer();
  inst_desc* descriptors = static_cast<inst_desc*>(descriptorBuffer->contents());
  for (int i = 0; i < meshes.size(); i+=1) {
    descriptors[i].transformationMatrix = EXP::MATH::pack(meshes[i]->f4x4()->get());
  }
  descriptor->setInstanceDescriptorBuffer(descriptorBuffer);
  return descriptor;
}

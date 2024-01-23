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
      DEBUG(
          "Layout: " + std::to_string(layoutIndex) + " Element: " + std::to_string(elementIndex) +
          ", Offset: " + std::to_string(stride)
      );

      MTL::VertexAttributeDescriptor* positionDescriptor =
          vertexDescriptor->attributes()->object(elementIndex);
      positionDescriptor->setFormat((MTL::VertexFormat)element.type);
      positionDescriptor->setOffset(stride);
      positionDescriptor->setBufferIndex(layoutIndex);
      stride += element.size;
      elementIndex += 1;
    }
    DEBUG("Layout: " + std::to_string(layoutIndex) + ", Stride: " + std::to_string(stride));
    MTL::VertexBufferLayoutDescriptor* layoutDescriptor =
        vertexDescriptor->layouts()->object(layoutIndex);
    layoutDescriptor->setStride(stride);
    layoutDescriptor->setStepRate(1);
    layoutDescriptor->setStepFunction(MTL::VertexStepFunctionPerVertex);
  }

  return vertexDescriptor;
}

Renderer::PDV Renderer::Descriptor::primitives(Explorer::Model* model) {
  PDV result;
  for (auto mesh : model->meshes)
    result.emplace_back(Descriptor::primitive(mesh));
  return result;
}

MTL::RenderPipelineDescriptor* Renderer::Descriptor::render(
    MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, std::string path
) {
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

  descriptor->setVertexDescriptor(vertexDescriptor);
  library->release();
  return descriptor;
}

MTL::ComputePipelineDescriptor*
Renderer::Descriptor::compute(MTL::Device* device, std::string path) {
  MTL::Library* library = Repository::Shaders::readLibrary(device, path);
  MTL::ComputePipelineDescriptor* descriptor = MTL::ComputePipelineDescriptor::alloc()->init();
  descriptor->setSupportAddingBinaryFunctions(true);
  descriptor->setComputeFunction(library->newFunction(Explorer::nsString("computeKernel")));
  library->release();
  return descriptor;
}

// For every mesh :: 1 PrimitiveAccelerationStructure
// For every submesh :: 1 Geometry
MTL::PrimitiveAccelerationStructureDescriptor* Renderer::Descriptor::primitive(Explorer::Mesh* mesh
) {
  std::vector<MTL::AccelerationStructureTriangleGeometryDescriptor*> dGeometries;
	std::vector<Explorer::Submesh*> submeshes = mesh->submeshes();

  for (int i = 0; i < mesh->count; i++) {
    MTL::AccelerationStructureTriangleGeometryDescriptor* dGeometry =
        MTL::AccelerationStructureTriangleGeometryDescriptor::descriptor();
		Explorer::Submesh* submesh = submeshes[i];
    
    dGeometry->setVertexBuffer(mesh->buffers[0]);
    dGeometry->setVertexBufferOffset(mesh->offsets[0]);

    // Q: Should I be using packed floats here?
    // For now: 4float x 4 (due to padding) == 64 bytes
    // Future: use layoutDescriptor instead
    dGeometry->setVertexStride(16);
    dGeometry->setIndexBuffer(submesh->indexBuffer);
    dGeometry->setIndexType(submesh->indexType);
    dGeometry->setIndexBufferOffset(submesh->offset);
    dGeometry->setTriangleCount(submesh->indexCount / 3);

    dGeometries.emplace_back(dGeometry);
  }

  MTL::PrimitiveAccelerationStructureDescriptor* dPrimitive =
      MTL::PrimitiveAccelerationStructureDescriptor::descriptor();
  NS::Array* aGeometries =
      NS::Array::array((NS::Object* const*)&dGeometries[0], dGeometries.size());
  dPrimitive->setGeometryDescriptors(aGeometries);
  return dPrimitive;
}

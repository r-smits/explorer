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
      //DEBUG(
      //    "Layout: " + std::to_string(layoutIndex) + " Element: " + std::to_string(elementIndex) +
      //    ", Offset: " + std::to_string(stride)
      //);
      MTL::VertexAttributeDescriptor* positionDescriptor =
          vertexDescriptor->attributes()->object(elementIndex);
      positionDescriptor->setFormat((MTL::VertexFormat)element.type);
      positionDescriptor->setOffset(stride);
      positionDescriptor->setBufferIndex(layoutIndex);
      stride += element.size;
      elementIndex += 1;
    }
    //DEBUG("Layout: " + std::to_string(layoutIndex) + ", Stride: " + std::to_string(stride));
    MTL::VertexBufferLayoutDescriptor* layoutDescriptor =
        vertexDescriptor->layouts()->object(layoutIndex);
    layoutDescriptor->setStride(stride);
    layoutDescriptor->setStepRate(1);
    layoutDescriptor->setStepFunction(MTL::VertexStepFunctionPerVertex);
  }

  return vertexDescriptor;
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

NS::Array* Renderer::Descriptor::primitives(const std::vector<Explorer::Model*>& scene, const int& stride) {
	std::vector<Explorer::Mesh*> meshes;
	for (Explorer::Model* model: scene) {
		for (Explorer::Mesh* mesh : model->meshes) meshes.emplace_back(mesh);	
	}
  std::vector<MTL::PrimitiveAccelerationStructureDescriptor*> descriptors;
  for (Explorer::Mesh* mesh : meshes)
    descriptors.emplace_back(Descriptor::primitive(mesh, stride));
  NS::Array* result = NS::Array::array((NS::Object* const*)&descriptors[0], descriptors.size());
  return result;
}

// For every mesh :: 1 PrimitiveAccelerationStructure, 1 instance
// For every submesh :: 1 Geometry
MTL::PrimitiveAccelerationStructureDescriptor*
Renderer::Descriptor::primitive(Explorer::Mesh* mesh, const int& stride) {
  std::vector<MTL::AccelerationStructureTriangleGeometryDescriptor*> geometryDescriptors;
  std::vector<Explorer::Submesh*> submeshes = mesh->submeshes();

  for (int i = 0; i < mesh->count; i++) {
    MTL::AccelerationStructureTriangleGeometryDescriptor* geometryDescriptor =
        MTL::AccelerationStructureTriangleGeometryDescriptor::descriptor();
    Explorer::Submesh* submesh = submeshes[i];

    geometryDescriptor->setVertexBuffer(mesh->buffers[0]);
    geometryDescriptor->setVertexBufferOffset(mesh->offsets[0]);

    // A: Using a packed float, set to 3float in buffer
    // Using non-interleaved vertex buffer
    geometryDescriptor->setVertexStride(stride);
    geometryDescriptor->setIndexBuffer(submesh->indexBuffer);
    geometryDescriptor->setIndexType(submesh->indexType);
    geometryDescriptor->setIndexBufferOffset(submesh->offset);
    geometryDescriptor->setTriangleCount(submesh->indexCount / 3);
    geometryDescriptors.emplace_back(geometryDescriptor);

    //DEBUG(
    //    "Built triangle geometry descriptor. Indices: " + std::to_string(submesh->indexCount) +
    //    " Triangles: " + std::to_string(geometryDescriptor->triangleCount())
    //);
  }

  MTL::PrimitiveAccelerationStructureDescriptor* dPrimitive =
      MTL::PrimitiveAccelerationStructureDescriptor::descriptor();
  NS::Array* aGeometries =
      NS::Array::array((NS::Object* const*)&geometryDescriptors[0], geometryDescriptors.size());
  dPrimitive->setGeometryDescriptors(aGeometries);

  //DEBUG(
  //    "Built primitive descriptor. Geometries: " +
  //    std::to_string(dPrimitive->geometryDescriptors()->count())
  //);
  return dPrimitive;
}

MTL::InstanceAccelerationStructureDescriptor* Renderer::Descriptor::instance(
    MTL::Device* device,
    NS::Array* primitiveStructures,
    const std::vector<Explorer::Model*>& scene
) {
	int count = 0;
	std::vector<Explorer::Mesh*> meshes;
	for (Explorer::Model* model : scene) {
		count += model->meshCount;
		for (Explorer::Mesh* mesh : model->meshes) meshes.emplace_back(mesh);
	}

  MTL::InstanceAccelerationStructureDescriptor* descriptor =
      MTL::InstanceAccelerationStructureDescriptor::descriptor();

  descriptor->setInstancedAccelerationStructures(primitiveStructures);
  descriptor->setInstanceCount(count);

  MTL::Buffer* instanceDescriptorBuffer = device->newBuffer(
      sizeof(MTL::AccelerationStructureInstanceDescriptor) * count,
      MTL::ResourceStorageModeShared
  );
  MTL::AccelerationStructureInstanceDescriptor* instanceDescriptors =
      (MTL::AccelerationStructureInstanceDescriptor*)instanceDescriptorBuffer->contents();

  for (int i = 0; i < count; i++) {
    instanceDescriptors[i].accelerationStructureIndex = i;
    instanceDescriptors[i].intersectionFunctionTableOffset = 0;
    instanceDescriptors[i].mask = 0xFF;
    instanceDescriptors[i].options = MTL::AccelerationStructureInstanceOptionNone;
    MTL::PackedFloat4x3 modelViewTransform = Transformation::pack(meshes[i]->f4x4()->get());
    instanceDescriptors[i].transformationMatrix = modelViewTransform;
  }
  descriptor->setInstanceDescriptorBuffer(instanceDescriptorBuffer);
  //DEBUG("Built instance structure descriptor. Instances: " + std::to_string(count));
  return descriptor;
}

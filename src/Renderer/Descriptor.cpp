#include <Renderer/Descriptor.h>

MTL::VertexDescriptor* Renderer::Descriptor::vertex(MTL::Device* device, BufferLayout* layout) {
  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  int i = 0;
  int stride = 0;
  for (BufferElement element : layout->getElements()) {
    MTL::VertexAttributeDescriptor* positionDescriptor = vertexDescriptor->attributes()->object(i);
    positionDescriptor->setFormat((MTL::VertexFormat)element.type);
    positionDescriptor->setOffset(stride);
    positionDescriptor->setBufferIndex(0);
    stride += element.size;
    i += 1;
  }

  MTL::VertexBufferLayoutDescriptor* layoutDescriptor = vertexDescriptor->layouts()->object(0);
  layoutDescriptor->setStride(stride);

  return vertexDescriptor;
}

#include <Renderer/Draw.h>
#include <View/ViewAdapter.hpp>

void Renderer::Draw::light(
    MTL::RenderCommandEncoder* encoder, EXP::Camera camera, EXP::Light* light
) {
  light->data.position = light->data.position;
  encoder->setFragmentBytes(
      &light->data,            // Setting a buffer
      sizeof(Renderer::Light), // Size of the data
      1                        // Index of the buffer
  );
}

void Renderer::Draw::model(
    MTL::RenderCommandEncoder* encoder, EXP::Camera camera, EXP::Model* model
) {
  // All calculations of the world are set only once and applied per vertex in
  // shader

  //Renderer::Projection projection = {{camera.get()}, {model->get()}, {camera.position}};
  //encoder->setVertexBytes(
  //    &projection,                  // The data set in GP
  //    sizeof(Renderer::Projection), // The size of data set in GPU
  //    20                             // The location of data: [[buffer(20)]]
  //);
	
	/**
  // For all meshes set vertex buffer, for all submeshes, index buffer and
  // materials
  for (EXP::Mesh* mesh : model->meshes) {
		
    for (int i = 0; i < mesh->bufferCount; i++) {
      encoder->setVertexBuffer(
          mesh->buffers[i], // The data to use for vertex buffer
          0,                // The offset of the vertex buffer
          i                 // The index in the buffer to start drawing from
      );
    }

    for (EXP::MDL::Submesh* submesh : mesh->submeshes()) {
      encoder->setFragmentBytes(&submesh->material, sizeof(Renderer::Material), 2);
      encoder->setFragmentTexture(submesh->textures[0],
                                  0); // Setting texture to buffer(0)

      encoder->drawIndexedPrimitives(
          submesh->primitiveType, // Type of object to draw
          submesh->indexCount,    // Number of elements in the index buffer
          submesh->indexType,     // The data type of the data in buffer
          submesh->indexBuffer,   // The index buffer holding the indice data
          submesh->offset,        // The index buffer offset
          NS::UInteger(1)         // For instanced rendering. We render 1 object
      );
    }
  }
	**/
}

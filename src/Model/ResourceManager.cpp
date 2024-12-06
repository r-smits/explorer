#include <Model/ResourceManager.h>

using namespace EXP;

const std::vector<MTL::Resource*>& SCENE::getResources() { return resources; };

const std::vector<MTL::Texture*>& SCENE::getTextures() { return textures; };

const std::vector<EXP::Model*>& SCENE::getModels() { return models; };

MTL::Buffer* SCENE::getBindlessScene() { return sceneBuffer; };

EXP::Model* SCENE::getModel(const std::string& name) {
	if (modnames.find(name) == modnames.end()) {
		DEBUG("Cannot find model.");
		return nullptr;
	}
	int& index = modnames[name];
	return models[index];
};

void SCENE::addModel(
    MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, const std::string& path, const std::string& name
) {
  EXP::Model* model = Repository::Meshes::read(device, vertexDescriptor, path);
  models.emplace_back(model);
	modnames.insert({name, models.size()-1});
	DEBUG("Model stored. Name: " + model->name);
};

const int& SCENE::addTexture(const Repository::TextureWithName& textureWithName) {
  const std::string& name = textureWithName.name;
  if (EXP::SCENE::texnames.find(name) == EXP::SCENE::texnames.end()) {
          EXP::SCENE::texcounter += 1;
          EXP::SCENE::texnames.insert({name, (int)EXP::SCENE::texcounter});
          EXP::SCENE::textures.emplace_back(textureWithName.texture);
          DEBUG("Texture stored. Index: " +
  std::to_string(EXP::SCENE::texcounter) + ", name: " + name); } else {
          DEBUG("Texture already stored. Index: " +
  std::to_string(EXP::SCENE::texnames[name]) + ", name: " + name);
  }
  return EXP::SCENE::texnames[name];
}

inline MTL::Texture* SCENE::getTexture(const std::string& name) {
  const int& index = texnames[name];
  return textures[index];
}

inline MTL::Texture* SCENE::getTexture(const int& index) { return textures[index]; }

const void SCENE::buildBindlessScene(MTL::Device* device) {
	
	int totalMeshCount = 0;
	std::vector<Mesh*> allMeshes;
	
	for (EXP::Model* model : models) {
			totalMeshCount += model->meshCount;
		for (Mesh* mesh : model->meshes) allMeshes.emplace_back(mesh);
	}
	
  int meshBufferSize = sizeof(Renderer::Mesh) * totalMeshCount;
  MTL::Buffer* meshBuffer = device->newBuffer(meshBufferSize, MTL::ResourceStorageModeShared);
  resources.emplace_back(meshBuffer);

  for (int j = 0; j < totalMeshCount; j++) {
    Renderer::Mesh* gpuMesh = ((Renderer::Mesh*)meshBuffer->contents()) + j;
    EXP::Mesh* cpuMesh = allMeshes[j];

    gpuMesh->vertices = cpuMesh->buffers[0]->gpuAddress() + cpuMesh->offsets[0];
    gpuMesh->attributes = cpuMesh->buffers[1]->gpuAddress() + cpuMesh->offsets[1];

    resources.emplace_back(cpuMesh->buffers[0]);
    resources.emplace_back(cpuMesh->buffers[1]);

    int submeshBufferSize = sizeof(Renderer::Submesh) * cpuMesh->count;
    MTL::Buffer* submeshBuffer = device->newBuffer(submeshBufferSize, MTL::ResourceStorageModeShared);
    resources.emplace_back(submeshBuffer);

    for (int k = 0; k < cpuMesh->count; k++) {
      Renderer::Submesh* gpuSubmesh = ((Renderer::Submesh*)submeshBuffer->contents()) + k;
			EXP::MDL::Submesh* cpuSubmesh = cpuMesh->submeshes()[k];

      gpuSubmesh->indices = cpuSubmesh->indexBuffer->gpuAddress() + cpuSubmesh->offset;
      gpuSubmesh->texture = cpuSubmesh->textures[0]->gpuResourceID();
			gpuSubmesh->textured = cpuSubmesh->material.useColor;
			gpuSubmesh->emissive = cpuSubmesh->material.useLight;

      resources.emplace_back(cpuSubmesh->indexBuffer);
      resources.emplace_back(cpuSubmesh->textures[0]);
    }

    gpuMesh->submeshes = submeshBuffer->gpuAddress();
  }

	sceneBuffer = device->newBuffer(sizeof(Renderer::Scene), MTL::ResourceStorageModeShared);
  resources.emplace_back(sceneBuffer);

  Renderer::Scene* gpuScene = ((Renderer::Scene*)sceneBuffer->contents());
  gpuScene->meshes = meshBuffer->gpuAddress();
};



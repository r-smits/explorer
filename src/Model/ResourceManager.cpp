#include "Log/Logger.h"
#include "Metal/MTLResource.hpp"
#include "Model/Camera.h"
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
  if (texnames.find(name) == texnames.end()) {
		texcounter += 1;
    texnames.insert({name, texcounter});
    textures.emplace_back(textureWithName.texture);
    DEBUG("Texture stored. Index: " +
		std::to_string(EXP::SCENE::texcounter) + ", name: " + name); 
	} else {
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


void SCENE::addCamera(EXP::VCamera* camera) {
	vcamera = camera;
};

EXP::VCamera* SCENE::getCamera() {
	return vcamera;
};

MTL::Buffer* SCENE::buildTextureBuffer(MTL::Device* device) {
	MTL::Buffer* textureBuffer = device->newBuffer(sizeof(Renderer::Text2D) * textures.size(), MTL::ResourceStorageModeShared);
	resources.emplace_back(textureBuffer);
	Renderer::Text2D* textureBufferPtr = (Renderer::Text2D*)textureBuffer->contents();
	for (int t = 0; t < textures.size(); t += 1) {
		if (t == texnames["default"]) {
			DEBUG("Default nullptr texture is skipped.");
			continue;
		}
		(textureBufferPtr + t)->value = textures[t]->gpuResourceID();
		resources.emplace_back(textures[t]);
	}
	return textureBuffer;
};


MTL::Buffer* SCENE::buildVCameraBuffer(MTL::Device* device) {
	vcameraBuffer = device->newBuffer(sizeof(Renderer::VCamera), MTL::ResourceStorageModeShared);
	resources.emplace_back(vcameraBuffer);
	Renderer::VCamera* vcameraPtr = (Renderer::VCamera*) vcameraBuffer->contents();

	const Renderer::VCamera& updatedVCamera = vcamera->update();
	vcameraPtr->vecOrigin = updatedVCamera.vecOrigin;
	vcameraPtr->resolution = updatedVCamera.resolution;
	vcameraPtr->orientation = updatedVCamera.orientation;
	return vcameraBuffer;
};


const void SCENE::buildBindlessScene(MTL::Device* device) {
	vcamera = new VCamera();
	sceneBuffer = device->newBuffer(sizeof(Renderer::Scene), MTL::ResourceStorageModeShared);
  resources.emplace_back(sceneBuffer);
  Renderer::Scene* gpuScene = (Renderer::Scene*)sceneBuffer->contents();
	gpuScene->textures = SCENE::buildTextureBuffer(device)->gpuAddress();
	gpuScene->vcamera = SCENE::buildVCameraBuffer(device)->gpuAddress();
	};


const void SCENE::updateBindlessScene(MTL::Device* device) {
	Renderer::VCamera* vcameraPtr = (Renderer::VCamera*)vcameraBuffer->contents();
	const Renderer::VCamera& updatedVCamera = vcamera->update();
	vcameraPtr->vecOrigin = updatedVCamera.vecOrigin;
	vcameraPtr->resolution = updatedVCamera.resolution;
	vcameraPtr->orientation = updatedVCamera.orientation;
}






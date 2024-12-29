#include "Log/Logger.h"
#include "Metal/MTLResource.hpp"
#include "Model/Camera.h"
#include "Model/Mesh.h"
#include "Renderer/Types.h"
#include <Model/ResourceManager.h>

using namespace EXP;

const std::vector<MTL::Resource*>& SCENE::getResources() { return resources; };

const std::vector<Renderer::Texture>& SCENE::getTextures() { return textSample; };

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

const int& SCENE::addTexture(const Renderer::Texture& texture) {
	if (texture.access == Renderer::TextureAccess::SAMPLE) {
		if (textSampleNames.find(texture.name) == textSampleNames.end()) {
			textsampleCounter += 1;
			textSampleNames.insert({texture.name, textsampleCounter});
			textSample.emplace_back(texture);
			DEBUG("Sample texture stored. Index: " +
			std::to_string(EXP::SCENE::textsampleCounter) + ", name: " + texture.name);
		} else {
			DEBUG("Texture already stored. Index: " +
			std::to_string(EXP::SCENE::textSampleNames[texture.name]) + ", name: " + texture.name);
		}
		return EXP::SCENE::textSampleNames[texture.name];
	} else {
		if (textReadWriteNames.find(texture.name) == textReadWriteNames.end()) {
			textreadwriteCounter += 1;
			textReadWriteNames.insert({texture.name, textreadwriteCounter});
			textReadWrite.emplace_back(texture);
			DEBUG("Read/Write texture stored. Index: " +
			std::to_string(EXP::SCENE::textreadwriteCounter) + ", name: " + texture.name);
		} else {
			DEBUG("Texture already stored. Index: " +
			std::to_string(EXP::SCENE::textReadWriteNames[texture.name]) + ", name: " + texture.name);
		}
		return EXP::SCENE::textReadWriteNames[texture.name];
	}
}

const int& SCENE::addTexture(MTL::Device* device, const std::string& name, const Renderer::TextureAccess& access) {
	MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(
			MTL::PixelFormat::PixelFormatRGBA16Float,
			2000, 
			1400, 
			false
	);
	MTL::Texture* mtlTexture = device->newTexture(textureDescriptor);
	Renderer::Texture texture = {name, access, mtlTexture}; 
	return EXP::SCENE::addTexture(texture);	
}

MTL::Texture* SCENE::getTexture(const std::string& name, Renderer::TextureAccess access) {
	if (access == Renderer::TextureAccess::SAMPLE) {
		const int& index = textSampleNames[name];
		return textSample[index].value;
	} else {
		const int& index = textReadWriteNames[name];
		return textReadWrite[index].value;
	}
}

MTL::Texture* SCENE::getTexture(const int& index, Renderer::TextureAccess access) { 
	if (access == Renderer::TextureAccess::SAMPLE) {
		return textSample[index].value;
	} else {
		return textReadWrite[index].value;
	}
}

void SCENE::addCamera(EXP::VCamera* camera) {
	vcamera = camera;
};

EXP::VCamera* SCENE::getCamera() {
	return vcamera;
};

MTL::Buffer* SCENE::buildTextSampleBuffer(MTL::Device* device) {
	DEBUG("Preparing sample texture buffer...");
	MTL::Buffer* textSampleBuffer = 
		device->newBuffer(sizeof(Renderer::Text2D) * textSample.size(), MTL::ResourceStorageModeShared);
	resources.emplace_back(textSampleBuffer);

	Renderer::Text2D* textureBufferPtr = (Renderer::Text2D*)textSampleBuffer->contents();
	for (int t = 0; t < textSample.size(); t += 1) {
		if (t == textSampleNames["default"]) {
			DEBUG("Default nullptr texture is skipped.");
			continue;
		}
		(textureBufferPtr + t)->value = textSample[t].value->gpuResourceID();
		resources.emplace_back(textSample[t].value);
	}
	return textSampleBuffer;
};

MTL::Buffer* SCENE::buildTextReadWriteBuffer(MTL::Device* device) {
	DEBUG("Preparing read/write texture buffer...");
	MTL::Buffer* textReadWriteBuffer = 
		device->newBuffer(sizeof(Renderer::Text2D) * textReadWrite.size(), MTL::ResourceStorageModeShared);
	resources.emplace_back(textReadWriteBuffer);

	Renderer::Text2D* textureBufferPtr = (Renderer::Text2D*)textReadWriteBuffer->contents();
	for (int t = 0; t < textReadWrite.size(); t += 1) {
		if (t == textReadWriteNames["default"]) {
			DEBUG("Default nullptr texture is skipped.");
			continue;
		}
		(textureBufferPtr + t)->value = textReadWrite[t].value->gpuResourceID();
		resources.emplace_back(textReadWrite[t].value);
	}
	return textReadWriteBuffer;
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

MTL::Buffer* SCENE::buildLightsBuffer(MTL::Device* device) {
	
	for (EXP::Model* model: SCENE::models) {
		if (model->isEmissive()) {
			DEBUG("Found emissive model.");
			for (EXP::MDL::Mesh* mesh : model->meshes) {
				SCENE::lights.emplace_back(mesh);
			}
		}
	}

	SCENE::lightsBuffer = device->newBuffer(sizeof(Renderer::Mesh) * lights.size(), MTL::ResourceStorageModeShared);
  resources.emplace_back(lightsBuffer);
	DEBUG("Number of lights: " + std::to_string(lights.size()));	
	for (int i = 0; i < lights.size(); i++) {
		Renderer::Mesh* gpuMesh = (Renderer::Mesh*)lightsBuffer->contents() + i;
		EXP::MDL::Mesh* cpuMesh = SCENE::lights[i];
		gpuMesh->vertices = cpuMesh->buffers[0]->gpuAddress() + cpuMesh->offsets[0];
		gpuMesh->attributes = cpuMesh->buffers[1]->gpuAddress() + cpuMesh->offsets[1];
		gpuMesh->orientation = cpuMesh->f4x4()->get();
		gpuMesh->vertexCount = cpuMesh->vertexCount;
		DEBUG("vertex count: " + std::to_string(cpuMesh->vertexCount));
		resources.emplace_back(cpuMesh->buffers[0]);
		resources.emplace_back(cpuMesh->buffers[1]);

		DEBUG("Number of submeshes: " + std::to_string(lights[i]->submeshes.size()));
		int submeshBufferSize = sizeof(Renderer::Submesh) * lights[i]->submeshes.size();
		MTL::Buffer* submeshBuffer = device->newBuffer(submeshBufferSize, MTL::ResourceStorageModeShared);
		resources.emplace_back(submeshBuffer);
		for (int k = 0; k < lights[i]->submeshes.size(); k++) {
			Renderer::Submesh* gpuSubmesh = (Renderer::Submesh*)submeshBuffer->contents() + k;
			EXP::MDL::Submesh* cpuSubmesh = cpuMesh->submeshes[k];
			gpuSubmesh->indices = cpuSubmesh->indexBuffer->gpuAddress() + cpuSubmesh->offset;
			resources.emplace_back(cpuSubmesh->indexBuffer);
		}
		gpuMesh->submeshes = submeshBuffer->gpuAddress();
	}
	return lightsBuffer;
};


const void SCENE::buildBindlessScene(MTL::Device* device) {
	vcamera = new VCamera();
	sceneBuffer = device->newBuffer(sizeof(Renderer::Scene), MTL::ResourceStorageModeShared);
  resources.emplace_back(sceneBuffer);
  Renderer::Scene* gpuScene = (Renderer::Scene*)sceneBuffer->contents();
	gpuScene->textsample = SCENE::buildTextSampleBuffer(device)->gpuAddress();
	gpuScene->textreadwrite = SCENE::buildTextReadWriteBuffer(device)->gpuAddress();
	gpuScene->vcamera = SCENE::buildVCameraBuffer(device)->gpuAddress();
	gpuScene->lights = SCENE::buildLightsBuffer(device)->gpuAddress();
	gpuScene->lightsCount = SCENE::lights.size();
};


const void SCENE::updateBindlessScene(MTL::Device* device) {
	Renderer::VCamera* vcameraPtr = (Renderer::VCamera*)vcameraBuffer->contents();
	const Renderer::VCamera& updatedVCamera = vcamera->update();
	vcameraPtr->vecOrigin = updatedVCamera.vecOrigin;
	vcameraPtr->resolution = updatedVCamera.resolution;
	vcameraPtr->orientation = updatedVCamera.orientation;

	Renderer::Mesh* meshPtr = (Renderer::Mesh*)lightsBuffer->contents();
	for (int i = 0; i < SCENE::lights.size(); i += 1) {
		(meshPtr + i)->orientation = SCENE::lights[i]->f4x4()->get();
	}
};


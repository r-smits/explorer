#pragma once
#include <pch.h>
#include <Model/Camera.h>
#include <DB/Repository.hpp>
#include <unordered_map>


namespace EXP {

	
class SCENE {

	static inline EXP::VCamera* vcamera;
	static inline MTL::Buffer* vcameraBuffer = nullptr;
	
	static inline std::vector<EXP::Model*> models = {};
	static inline std::unordered_map<std::string, int> modnames = {};
	static inline MTL::Buffer* sceneBuffer = nullptr;

	static inline std::vector<MTL::Resource*> resources = {};
	static inline std::unordered_map<std::string, int> resnames = {};

	static inline std::vector<Renderer::Texture> textSample = {};
	static inline std::vector<Renderer::Texture> textReadWrite = {};
	static inline std::unordered_map<std::string, int> textSampleNames = {};
	static inline std::unordered_map<std::string, int> textReadWriteNames = {};

	static inline int textsampleCounter = -1;
	static inline int textreadwriteCounter = -1;

	static inline std::vector<EXP::MDL::Mesh*> lights = {};
	static inline MTL::Buffer* lightsBuffer = nullptr;

public:
	SCENE(){};
  ~SCENE(){};

	static const std::vector<MTL::Resource*>& getResources(); 
		
	static void addModel(
			MTL::Device* device, 
			MTL::VertexDescriptor* vertexDescriptor, 
			const std::string& path, 
			const std::string& name
	);
	static void addModel(EXP::Model* model);
	static EXP::Model* getModel(const std::string& name);
	static const std::vector<EXP::Model*>& getModels();

	static const int& addTexture(const Renderer::Texture& texture);
	static const int& addTexture(MTL::Device* device, const std::string& name, const Renderer::TextureAccess& access);
	static MTL::Texture* getTexture(const std::string& name, Renderer::TextureAccess access);
	static MTL::Texture* getTexture(const int& index, Renderer::TextureAccess access);
	static const std::vector<Renderer::Texture>& getTextures(); 
	
	static void addCamera(EXP::VCamera* camera);
	static EXP::VCamera* getCamera();

	static const void buildBindlessScene(MTL::Device* device);
	static const void updateBindlessScene(MTL::Device* device);
	static MTL::Buffer* getBindlessScene();

	static MTL::Buffer* buildTextSampleBuffer(MTL::Device* device);
	static MTL::Buffer* buildTextReadWriteBuffer(MTL::Device* device);
	static MTL::Buffer* buildVCameraBuffer(MTL::Device* device);
	static MTL::Buffer* buildLightsBuffer(MTL::Device* device);



};
}



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

	static inline std::vector<MTL::Texture*> textures = {};
	static inline std::unordered_map<std::string, int> texnames = {};
	static inline int texcounter = -1;

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

	static const int& addTexture(const Repository::TextureWithName& textureWithName);
	static const int& addTexture(MTL::Device* device, const std::string& name);
	static MTL::Texture* getTexture(const std::string& name);
	static MTL::Texture* getTexture(const int& index);
	static const std::vector<MTL::Texture*>& getTextures(); 
	
	static void addCamera(EXP::VCamera* camera);
	static EXP::VCamera* getCamera();

	static const void buildBindlessScene(MTL::Device* device);
	static const void updateBindlessScene(MTL::Device* device);
	static MTL::Buffer* getBindlessScene();

	static MTL::Buffer* buildTextureBuffer(MTL::Device* device);
	static MTL::Buffer* buildVCameraBuffer(MTL::Device* device);



};
}



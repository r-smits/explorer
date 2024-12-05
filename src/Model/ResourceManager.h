#pragma once
#include <DB/Repository.hpp>
#include <unordered_map>


namespace EXP {

	
class SCENE {
	
	static inline std::vector<EXP::Model*> models = {};
	static inline std::unordered_map<std::string, int> modnames = {};

	static inline std::vector<MTL::Resource*> resources = {};
	static inline std::unordered_map<std::string, int> resnames = {};

	static inline std::vector<MTL::Texture*> textures = {};
	static inline std::unordered_map<std::string, int> texnames = {};
	static inline int texcounter = 0;

public:
	SCENE(){};
  ~SCENE(){};

	static const std::vector<MTL::Resource*>& getResources(); 
	static const std::vector<MTL::Texture*>& getTextures(); 
	static const std::vector<EXP::Model*>& getModels();

	static void addModel(
			MTL::Device* device, 
			MTL::VertexDescriptor* vertexDescriptor, 
			const std::string& path, 
			const std::string& name
	);
	static void addModel(EXP::Model* model);
	static EXP::Model* getModel(const std::string& name);
	static EXP::Model* getModel(const int& index);

	static const int& addTexture(const Repository::TextureWithName& textureWithName);
	static MTL::Texture* getTexture(const std::string& name);
	static MTL::Texture* getTexture(const int& index);

	static const void buildBindlessScene(MTK::View* view); 
	static MTL::Buffer* getBindlessScene(MTK::View* view); 

};
}



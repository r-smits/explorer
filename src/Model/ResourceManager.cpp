#include <Model/ResourceManager.h>

using namespace EXP;

const std::vector<MTL::Resource*>& SCENE::getResources() { return resources; };

const std::vector<MTL::Texture*>& SCENE::getTextures() { return textures; };

const std::vector<EXP::Model*>& SCENE::getModels() { return models; };

void SCENE::addModel(
    MTL::Device* device, MTL::VertexDescriptor* vertexDescriptor, const std::string& path, const std::string& name
) {
  EXP::Model* model = Repository::Meshes::read(device, vertexDescriptor, path);
  models.emplace_back(model);
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

inline void buildBindlessScene(MTK::View* view){

};

inline MTL::Buffer* getBindlessScene(MTK::View* view) { return nullptr; };

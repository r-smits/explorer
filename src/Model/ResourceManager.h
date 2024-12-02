#pragma once
#import <pch.h>
#import <Model/MeshFactory.h>



namespace EXP {

class Resource {

	inline const std::vector<MTL::Resource*> getResources();
	inline const std::vector<MTL::Texture*>& getTextures();
	inline const std::vector<EXP::Model*>& getMeshes();

	inline const void addMesh(const std::string& meshName);
	inline const void buildBindlessScene(MTK::View* view);
	inline const MTL::Buffer* getBindlessScene(MTK::View* view); 
};
}



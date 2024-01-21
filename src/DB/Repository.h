#pragma once
#include <DB/Repository.h>
#include <ModelIO/ModelIO.h>
#include <Renderer/Types.h>
#include <pch.h>

@interface TextureRepository : NSObject {
}

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material;
+ (MTL::Texture*)read:(MTL::Device*)device
             material:(MDLMaterial*)material
             semantic:(MDLMaterialSemantic)semantic;
+ (MTL::Texture*)read:(MTL::Device*)device material:(MDLMaterial*)material;
@end

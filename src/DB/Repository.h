#pragma once
#include <Renderer/Types.h>
#include <DB/Repository.h>
#include <ModelIO/ModelIO.h>
#include <pch.h>

@interface TextureRepository : NSObject {
}

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material;
+ (MTL::Texture*)read:(MTL::Device*)device
             semantic:(MDLMaterialSemantic)semantic
             material:(MDLMaterial*)material;
+ (MTL::Texture*)read:(MTL::Device*)device material:(MDLMaterial*)material;
@end

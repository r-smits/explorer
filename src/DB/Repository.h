#pragma once
#include <DB/Repository.hpp>
#include <ModelIO/ModelIO.h>
#include <Renderer/Types.h>
#include <pch.h>
#include <util.h>
#include <MetalKit/MetalKit.h>
#include <Model/MeshFactory.h>
#include <Model/ResourceManager.h>


@interface TextureRepository : NSObject {
}

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material;
+ (MTL::Texture*)read:(MTL::Device*)device semantic:(MDLMaterialSemantic)semantic material:(MDLMaterial*)material;
+ (Renderer::Texture)read:(MTL::Device*)device material:(MDLMaterial*)material;

@end

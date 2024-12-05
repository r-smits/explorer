#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <MetalKit/MTKTextureLoader.h>
#include <ModelIO/MDLAssetResolver.h>
#include <ModelIO/MDLMaterial.h>
#include <ModelIO/ModelIO.h>
#include <Model/ResourceManager.h>

@implementation TextureRepository

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material {
  Renderer::Material result = {
      {1.0f, 1.0f, 1.0f, 1.0f},
      {[material propertyWithSemantic:MDLMaterialSemanticEmission].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticRoughness].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticSpecular].float4Value},
  };
  result.specular.w = [material propertyWithSemantic:MDLMaterialSemanticSpecularExponent].floatValue;
  return result;
}

+ (MTL::Texture*)read:(MTL::Device*)device semantic:(MDLMaterialSemantic)semantic material:(MDLMaterial*)material {

  MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  MDLMaterialProperty* property = [material propertyWithSemantic:semantic];
  MDLTextureSampler* sampler = [property textureSamplerValue];
  if (!sampler) WARN("No sampler found");

  MDLTexture* texture = [property textureSamplerValue].texture;
  if (!texture) WARN("No texture found");

  NSDictionary* options = @{
    MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft,
    MTKTextureLoaderOptionGenerateMipmaps : @true
  };

  NSError* error = nil;
  id<MTLTexture> mtlTexture = [loader newTextureWithMDLTexture:texture options:options error:&error];
  if (error) EXP::printError((__bridge NS::Error*)error);
  return (__bridge MTL::Texture*)mtlTexture;
}

+ (Repository::TextureWithName)read:(MTL::Device*)device material:(MDLMaterial*)material {
	
	id<MDLAssetResolver> assetResolver = nullptr;
	[material loadTexturesUsingResolver:assetResolver];
  NSArray<MDLMaterialProperty*>* properties = [material propertiesWithSemantic:MDLMaterialSemanticBaseColor];
  MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];
	
  NSDictionary* options = @{
    MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
    MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
    MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft,
    MTKTextureLoaderOptionGenerateMipmaps : @true
  };
	
	NSError* err = nil;
  for (MDLMaterialProperty* property : properties) {
		if (property.type == MDLMaterialPropertyTypeTexture) {
			
			std::string filename = [[property.URLValue relativeString] UTF8String];
			filename = filename.substr(filename.find_last_of("/")+1);
			
			MDLTextureSampler* sampler = [property textureSamplerValue];
			MDLTexture* texture = [property textureSamplerValue].texture;
			id<MTLTexture> mtlTexture = [loader newTextureWithMDLTexture:texture options:options error:&err];

			if (err) EXP::printError((__bridge NS::Error*)err);
			MTL::Texture* resultTexture = (__bridge MTL::Texture*)mtlTexture;
			return {resultTexture, filename};
		}
	}
  WARN("No texture found.");
  return {nullptr, "default"};
}

@end

MTL::Texture* Repository::Textures::read(MTL::Device* device, std::string path) {
  NSError* error = nil;
  MTKTextureLoader* loader = [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];
  NSURL* fullPath = (__bridge NSURL*)EXP::nsUrl(path);
  NSDictionary* options = @{MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft};
  id<MTLTexture> texture = [loader newTextureWithContentsOfURL:fullPath options:options error:&error];
  return (__bridge MTL::Texture*)texture;
}

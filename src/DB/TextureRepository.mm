#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <MetalKit/MTKTextureLoader.h>
#include <ModelIO/ModelIO.h>

@implementation TextureRepository

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material {
  Renderer::Material result = {
      {1.0f, 1.0f, 1.0f, 1.0f},
      {[material propertyWithSemantic:MDLMaterialSemanticEmission].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticRoughness].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticSpecular].float4Value},
  };
  result.specular.w =
      [material propertyWithSemantic:MDLMaterialSemanticSpecularExponent].floatValue;
  return result;
}

+ (MTL::Texture*)read:(MTL::Device*)device material:(MDLMaterial*)material {
  return [TextureRepository read:device semantic:MDLMaterialSemanticBaseColor material:material];
}

+ (MTL::Texture*)read:(MTL::Device*)device
             semantic:(MDLMaterialSemantic)semantic
             material:(MDLMaterial*)material {

  MTKTextureLoader* loader =
      [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];

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
  id<MTLTexture> mtlTexture = [loader newTextureWithMDLTexture:texture
                                                       options:options
                                                         error:&error];
  if (error) Explorer::printError((__bridge NS::Error*)error);
  return (__bridge MTL::Texture*)mtlTexture;
}

+ (MTL::Texture*)read2:(MTL::Device*)device
              material:(MDLMaterial*)material
              semantic:(MDLMaterialSemantic)semantic {
	
	MTL::Texture* texture = nullptr;
	std::string name = "";

  NSArray<MDLMaterialProperty*>* properties = [material propertiesWithSemantic:semantic];
  MTKTextureLoader* loader =
      [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];
	
  NSDictionary* options = @{
    MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
    MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
    MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft,
    MTKTextureLoaderOptionGenerateMipmaps : @true
  };
	
	NSError* err = nil;
  for (MDLMaterialProperty* property : properties) {
    
		assert(property.semantic == semantic);
    std::string propertyURL = "/Users/ramonsmits/Code/cpp/graphics/Explorer/src/Assets/Meshes/f16/";
		name = [property.name UTF8String];
		
		if (property.type == MDLMaterialPropertyTypeFloat3) {
			DEBUG("Found float3");
		}

		if (property.type != MDLMaterialPropertyTypeString) {
			continue;
		}

    if (property.type == MDLMaterialPropertyTypeURL) {
      propertyURL = [[property.URLValue absoluteString] UTF8String];
    } else {
      std::string stringValue = [property.stringValue UTF8String];
      propertyURL.append(stringValue);
      DEBUG(name + ": Loading texture from: " + stringValue);
    }

    NSURL* textureURL = (__bridge NSURL*)Explorer::nsUrl(propertyURL);
    texture = (__bridge MTL::Texture*)[loader newTextureWithContentsOfURL:textureURL options:options error:&err];
    if (err) Explorer::printError((__bridge NS::Error*)err);
		if (texture) break;

    NSString* lastComponent = [[property.stringValue componentsSeparatedByString:@"/"] lastObject];
		std::string lastComponentStr = [lastComponent UTF8String];
    texture = (__bridge MTL::Texture*)[loader newTextureWithName:lastComponent
                             scaleFactor:1.0
                                  bundle:nil
                                 options:options
                                   error:&err];
		if (err) Explorer::printError((__bridge NS::Error*)err);
    
		if (texture) {
			DEBUG(name + ": Found file catalogue texture from: " + lastComponentStr);
			break;
		}
	}
  if (!texture) WARN(name + ": No texture found.");
  return texture;
}

@end

MTL::Texture* Repository::Textures::read(MTL::Device* device, std::string path) {
  NSError* error = nil;
  MTKTextureLoader* loader =
      [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];
  NSURL* fullPath = (__bridge NSURL*)Explorer::nsUrl(path);

  NSDictionary* options = @{MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft};

  id<MTLTexture> texture = [loader newTextureWithContentsOfURL:fullPath
                                                       options:options
                                                         error:&error];
  return (__bridge MTL::Texture*)texture;
}

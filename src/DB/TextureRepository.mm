#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include <MetalKit/MTKTextureLoader.h>
#include <ModelIO/ModelIO.h>
#include <util.h>

@implementation TextureRepository

+ (Renderer::Material)readMaterial:(MTL::Device*)device material:(MDLMaterial*)material {
  Renderer::Material result = {
      {1.0f, 1.0f, 1.0f, 1.0f},
      {[material propertyWithSemantic:MDLMaterialSemanticEmission].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticBaseColor].float3Value},
      {[material propertyWithSemantic:MDLMaterialSemanticSpecular].float4Value},
  };
  result.specular.w =
      [material propertyWithSemantic:MDLMaterialSemanticSpecularExponent].floatValue;
  return result;
}

+ (MTL::Texture*)read:(MTL::Device*)device material:(MDLMaterial*)material {
  return [TextureRepository read:device material:material semantic:MDLMaterialSemanticBaseColor];
}

+ (MTL::Texture*)read2:(MTL::Device*)device
              material:(MDLMaterial*)material
              semantic:(MDLMaterialSemantic)semantic {

  id<MTLTexture> texture = nullptr;
  NSArray<MDLMaterialProperty*>* properties = [material propertiesWithSemantic:semantic];
  MTKTextureLoader* loader =
      [[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device];

  NSDictionary* options = @{
    MTKTextureLoaderOptionTextureUsage : @(MTLTextureUsageShaderRead),
    MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
    MTKTextureLoaderOptionOrigin : MTKTextureLoaderOriginBottomLeft,
    MTKTextureLoaderOptionGenerateMipmaps : @true
  };

  for (MDLMaterialProperty* property : properties) {
    assert(property.semantic == semantic);
    std::string propertyURL = "file:///Users/ramonsmits/Code/Explorer/src/Assets/Meshes/f16/";
    if (property.type != MDLMaterialPropertyTypeString) continue;
    if (property.type == MDLMaterialPropertyTypeURL) {
      propertyURL = [[property.URLValue absoluteString] UTF8String];
    } else {
      std::string stringValue = [property.stringValue UTF8String];
      propertyURL.append(stringValue);
      DEBUG("Loading texture from: " + stringValue);
    }
    NSURL* textureURL = (__bridge NSURL*)Explorer::nsUrl(propertyURL);

    texture = [loader newTextureWithContentsOfURL:textureURL options:options error:nil];
    if (texture) return (__bridge MTL::Texture*)texture;

    // Interpret URL as file catalogue
    NSString* lastComponent = [[property.stringValue componentsSeparatedByString:@"/"] lastObject];
    texture = [loader newTextureWithName:lastComponent
                             scaleFactor:1.0
                                  bundle:nil
                                 options:options
                                   error:nil];
    if (texture) return (__bridge MTL::Texture*)texture;

    // Raise exception if not found in either case
    [NSException raise:@"Texture data for material property not found"
                format:@"Requested material property semantic: %lu string:%@",
                       semantic,
                       property.stringValue];
  }
  if (!texture) WARN("No texture found for semantic ");
  return (__bridge MTL::Texture*)texture;
}

+ (MTL::Texture*)read:(MTL::Device*)device
             material:(MDLMaterial*)material
             semantic:(MDLMaterialSemantic)semantic {

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

/**
[AAPLSubmesh createMetalTextureFromMaterial:modelIOSubmesh.material
                            modelIOMaterialSemantic:MDLMaterialSemanticTangentSpaceNormal
                                modelIOMaterialType:MDLMaterialPropertyTypeNone
                              metalKitTextureLoader:textureLoader];
**/

#include <DB/Repository.h>
#include <DB/Repository.hpp>
#include <Foundation/Foundation.h>
#include <MetalKit/MTKTextureLoader.h>
#include <ModelIO/ModelIO.h>

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

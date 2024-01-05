#include <DB/Repository.h>
#include <Foundation/Foundation.h>
#include <MetalKit/MTKTextureLoader.h>

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

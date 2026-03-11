#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Control/AppDelegate.h>
#include <Control/AppProperties.h>
#include <Events/KeyEvent.h>
#include <pch.h>


using path = std::filesystem::path;

int main() {
  NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

  path base_dir = std::filesystem::current_path() / "src";
  path shader_path = base_dir / "shaders";
  path texture_path = base_dir / "assets" / "textures";
  path mesh_path = base_dir / "assets" / "meshes";
    
  std::shared_ptr<const EXP::AppProperties> properties = std::make_shared<const EXP::AppProperties>(
      1512.0f,
      825.0f,
      shader_path.string(),
      texture_path.string(),
			mesh_path.string()
  );

  EXP::AppDelegate appDelegate = EXP::AppDelegate(properties);
  NS::Application* app = NS::Application::sharedApplication();

  app->setDelegate(&appDelegate);
  app->run();

  autoreleasePool->release();
  return 0;
}

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Control/AppDelegate.h>
#include <Control/AppProperties.h>
#include <Events/KeyEvent.h>
#include <pch.h>

int main() {
  NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

  Explorer::AppProperties properties = Explorer::AppProperties(
      1000.0f,
      700.0f,
      "/Users/ramonsmits/Code/Explorer/src/Shaders/",
      "/Users/ramonsmits/Code/Explorer/src/Assets/Textures/"
  );

  Explorer::AppDelegate appDelegate = Explorer::AppDelegate(&properties);
  NS::Application* app = NS::Application::sharedApplication();

  app->setDelegate(&appDelegate);
  app->run();

  autoreleasePool->release();
  return 0;
}

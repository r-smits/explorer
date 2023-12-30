#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Control/AppProperties.h>
#include <Control/AppDelegate.h>
#include <Events/KeyEvent.h>
#include <pch.h>


int main() {
	Explorer::KeyPressedEvent k("a", 0);
	DEBUG(k.toString());

  NS::AutoreleasePool *autoreleasePool = NS::AutoreleasePool::alloc()->init();
	
	Explorer::AppProperties properties = Explorer::AppProperties(500.0, 500.0);	
	Explorer::AppDelegate appDelegate = Explorer::AppDelegate(&properties);
  NS::Application *app = NS::Application::sharedApplication();

  app->setDelegate(&appDelegate);
  app->run();

  autoreleasePool->release();
  return 0;
}



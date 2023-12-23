#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <config.h>
#include <Control/AppDelegate.h>

int main() {

    void* libHandle = dlopen("./Dylib/test.dylib", RTLD_NOW);

    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    Explorer::AppDelegate controller;
    NS::Application* app = NS::Application::sharedApplication();

    app->setDelegate(&controller);
    app->run();

    autoreleasePool->release();
    return 0;
}

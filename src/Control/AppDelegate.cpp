#include <Control/AppDelegate.h>

Explorer::AppDelegate::~AppDelegate() {
    mtkView->release();
    window->release();
    device->release();
    delete viewDelegate;
}

void Explorer::AppDelegate::applicationWillFinishLaunching(NS::Notification* notification) {
    NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
    app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void Explorer::AppDelegate::applicationDidFinishLaunching(NS::Notification* notification) {
    CGRect frame = (CGRect) { {100.0, 100.0}, {1512.0, 850.0} };

    this->window = NS::Window::alloc()->init(
        frame,
        NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
        NS::BackingStoreBuffered,
        false
    );
    this->device = MTL::CreateSystemDefaultDevice();
    this->mtkView = MTK::View::alloc()->init(frame, this->device);
    this->mtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
    this->mtkView->setClearColor(MTL::ClearColor::Make(1.0, 1.0, 1.0, 1.0));

    this->viewDelegate = new ViewDelegate(device);
    mtkView->setDelegate(this->viewDelegate);

    window->setContentView(mtkView);
    window->setTitle(NS::String::string("Window", NS::StringEncoding::UTF8StringEncoding));
    window->makeKeyAndOrderFront(nullptr);

    NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
    app->activateIgnoringOtherApps(true);
}

bool Explorer::AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) {
    return true;
}
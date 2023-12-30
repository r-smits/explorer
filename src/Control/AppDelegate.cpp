#include <Control/AppDelegate.h>
#include <sstream>

Explorer::AppDelegate::AppDelegate(Explorer::AppProperties *properties) {
  this->properties = properties;
  ViewAdapter *viewAdapter = ViewAdapter::sharedInstance();
  this->mtkView = viewAdapter->getView(properties->cgRect);
}

Explorer::AppDelegate::~AppDelegate() {
  mtkView->release();
  window->release();
  device->release();
  delete viewDelegate;
}

double Explorer::AppDelegate::getWidth() { return properties->cgRect.size.width; }

double Explorer::AppDelegate::getHeight() { return properties->cgRect.size.height; }

void Explorer::AppDelegate::printDebug() {
  std::stringstream ss;
  ss << "Initialized view (" << this->getWidth() << "x" << this->getHeight() << ")";
  DEBUG(ss.str());
	ss.clear();
  ss << "FPS (" << this->mtkView->preferredFramesPerSecond() << ")";
  DEBUG(ss.str());
	ss.clear();
}

void Explorer::AppDelegate::applicationWillFinishLaunching(NS::Notification *msg) {
  NS::Application *app = reinterpret_cast<NS::Application *>(msg->object());
  app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void Explorer::AppDelegate::applicationDidFinishLaunching(NS::Notification *msg) {
  this->window = NS::Window::alloc()->init(
      properties->cgRect, NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
      NS::BackingStoreBuffered, false);

  // Set gpu for view to render with
  this->device = MTL::CreateSystemDefaultDevice();
  this->mtkView->setDevice(this->device);

  // Set MTK::View defaults
  this->mtkView->setPreferredFramesPerSecond((NS::Integer)120);
  this->mtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  this->mtkView->setClearColor(MTL::ClearColor::Make(1.0, 1.0, 1.0, 1.0));

  // Set object to be the MTK::View event handler
  this->viewDelegate = new ViewDelegate(device);
  this->mtkView->setDelegate(this->viewDelegate);

  // Set NS::Window defaults
  this->window->setContentView(mtkView);
  this->window->setTitle(NS::String::string("Window", NS::StringEncoding::UTF8StringEncoding));
  this->window->makeKeyAndOrderFront(nullptr);

  NS::Application *app = reinterpret_cast<NS::Application *>(msg->object());
  app->activateIgnoringOtherApps(true);
}

bool Explorer::AppDelegate::applicationShouldTerminateAfterLastWindowClosed(
    NS::Application *sender) {
  return true;
}

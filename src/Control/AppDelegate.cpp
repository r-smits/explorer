#include <Control/AppDelegate.h>
#include <sstream>

EXP::AppDelegate::AppDelegate(EXP::AppProperties* properties) {
  this->properties = properties;
	EXP::ViewAdapter* viewAdapter = ViewAdapter::sharedInstance();
  this->mtkView = ViewAdapter::initView(properties->cgRect);
}

EXP::AppDelegate::~AppDelegate() {
  mtkView->release();
  window->release();
  device->release();
  delete viewDelegate;
}

double EXP::AppDelegate::getWidth() { return properties->cgRect.size.width; }

double EXP::AppDelegate::getHeight() { return properties->cgRect.size.height; }

void EXP::AppDelegate::printDebug() {
  std::stringstream ss;
  ss << "Initialized view (" << this->getWidth() << "x" << this->getHeight() << ")";
  DEBUG(ss.str());
  ss.clear();
  ss << "FPS (" << this->mtkView->preferredFramesPerSecond() << ")";
  DEBUG(ss.str());
  ss.clear();
}

void EXP::AppDelegate::applicationWillFinishLaunching(NS::Notification* msg) {
  NS::Application* app = reinterpret_cast<NS::Application*>(msg->object());
  app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void EXP::AppDelegate::applicationDidFinishLaunching(NS::Notification* msg) {
  this->window = NS::Window::alloc()->init(
      properties->cgRect,
      NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
      NS::BackingStoreBuffered,
      false
  );

  // Set gpu for view to render with
  this->device = MTL::CreateSystemDefaultDevice();
  if (!device->supportsFamily(MTL::GPUFamily::GPUFamilyMetal3)) WARN("Metal 3 support required!");
  
	// Set MTK::View defaults
	mtkView->setPreferredFramesPerSecond((NS::Integer)60);
	mtkView->setEnableSetNeedsDisplay(true);
	mtkView->setFramebufferOnly(false); // Needed for compute kernel shaders
	mtkView->setPaused(false);
  
	mtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  mtkView->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));
  mtkView->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);
	mtkView->setDevice(this->device);

	// Set object to be the MTK::View event handler
  this->viewDelegate = new EXP::ViewDelegate(this->mtkView, this->properties);
  this->mtkView->setDelegate(this->viewDelegate);

  // Set NS::Window defaults
  this->window->setContentView(mtkView);
  this->window->setTitle(NS::String::string("EXPLORER", NS::StringEncoding::UTF8StringEncoding));
  this->window->makeKeyAndOrderFront(nullptr);

  NS::Application* app = reinterpret_cast<NS::Application*>(msg->object());
  app->activateIgnoringOtherApps(true);
}

bool EXP::AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender
) {
  return true;
}

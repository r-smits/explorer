#include <Layer/ImGuiLayer.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <View/ViewExtender.h>
#include <imgui.h>
#include <imgui_impl_metal.h>

Explorer::ImGuiLayer::ImGuiLayer(MTK::View *view)
    : Layer(view->device(), "ImGuiLayer") {
  this->onAttach(view);
}
Explorer::ImGuiLayer::~ImGuiLayer() {}

// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void Explorer::ImGuiLayer::onAttach(MTK::View *view) {
  DEBUG("Initializing ImGui ...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init((__bridge id<MTLDevice>)this->device);
}

void Explorer::ImGuiLayer::onDetach() {
  ImGui_ImplMetal_Shutdown();
  ImGui::DestroyContext();
}

void Explorer::ImGuiLayer::onUpdate(MTK::View *pView,
                                    MTL::RenderCommandEncoder *encoder) {

  MTL::CommandBuffer *buffer = this->queue->commandBuffer();
  MTL::RenderPassDescriptor *descriptor = pView->currentRenderPassDescriptor();

  MTKView *view = (__bridge MTKView *)pView;
  ImGuiIO &io = ImGui::GetIO();

  io.DisplaySize.x = view.bounds.size.width;
  io.DisplaySize.y = view.bounds.size.height;
  CGFloat scale = view.window.screen.backingScaleFactor
                      ?: NSScreen.mainScreen.backingScaleFactor;
  io.DisplayFramebufferScale = ImVec2(scale, scale);

  ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor *)descriptor);
  ImGui::NewFrame();

  static bool showDemo = true;
  ImGui::ShowDemoWindow(&showDemo);
  ImGui::Render();

  ImDrawData *drawData = ImGui::GetDrawData();
  ImGui_ImplMetal_RenderDrawData(drawData,
                                 (__bridge id<MTLCommandBuffer>)buffer,
                                 (__bridge id<MTLRenderCommandEncoder>)encoder);
}

void Explorer::ImGuiLayer::onEvent(Event &event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<MouseButtonReleasedEvent>(
      BIND_EVENT(ImGuiLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(
      BIND_EVENT(ImGuiLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(ImGuiLayer::onMouseMove));
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(ImGuiLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(ImGuiLayer::onKeyReleased));
}

bool Explorer::ImGuiLayer::onMouseButtonPressed(
    MouseButtonPressedEvent &event) {
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), true);
	event.done();
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onMouseButtonReleased(
    MouseButtonReleasedEvent &event) {
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), false);
	event.done();
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onMouseMove(MouseMoveEvent &event) {
  ImGuiIO &io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMousePosEvent(event.getX(), event.getY());
	event.done();
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onKeyPressed(KeyPressedEvent &event) {
  // Not implemented yet
  return true;
}

bool Explorer::ImGuiLayer::onKeyReleased(KeyReleasedEvent &event) {
  // Not implemented yet
  return true;
}



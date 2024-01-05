#include <Layer/ImGuiLayer.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <View/ViewExtender.h>
#include <imgui.h>
#include <imgui_impl_metal.h>

Explorer::ImGuiLayer::ImGuiLayer(MTK::View* view, AppProperties* config) : Layer(view->device(), config, "ImGuiLayer") {
  this->onAttach(view);
}
Explorer::ImGuiLayer::~ImGuiLayer() {}

// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void Explorer::ImGuiLayer::onAttach(MTK::View* view) {
  DEBUG("Initializing ImGui ...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init((__bridge id<MTLDevice>)this->device);
}

void Explorer::ImGuiLayer::onDetach() {
  ImGui_ImplMetal_Shutdown();
  ImGui::DestroyContext();
}

void Explorer::ImGuiLayer::onUpdate(MTK::View* pView, MTL::RenderCommandEncoder* encoder) {

  MTL::CommandBuffer* buffer = this->queue->commandBuffer();
  MTL::RenderPassDescriptor* descriptor = pView->currentRenderPassDescriptor();

  MTKView* view = (__bridge MTKView*)pView;
  ImGuiIO& io = ImGui::GetIO();

  io.DisplaySize.x = view.bounds.size.width;
  io.DisplaySize.y = view.bounds.size.height;
  CGFloat scale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
  io.DisplayFramebufferScale = ImVec2(scale, scale);

  ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor*)descriptor);
  ImGui::NewFrame();

  static bool showDemo = true;
  this->showDebugWindow(&showDemo);
  ImGui::Render();

  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplMetal_RenderDrawData(
      drawData, (__bridge id<MTLCommandBuffer>)buffer, (__bridge id<MTLRenderCommandEncoder>)encoder
  );
}

void Explorer::ImGuiLayer::showDebugWindow(bool* open) {

  static int location = 0;
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                  ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_NoSavedSettings |
                                  ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  const float PAD = 10.0f;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_pos = viewport->WorkPos;
  ImVec2 work_size = viewport->WorkSize;
  ImVec2 window_pos, window_pos_pivot;
  window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
  window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
  window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
  window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  window_flags |= ImGuiWindowFlags_NoMove;
  ImGui::SetNextWindowBgAlpha(0.80f);

  if (ImGui::Begin("DBWindow", open, window_flags)) {
    ImGui::Text("//// Debug information ////");
    ImGui::Separator();
    if (ImGui::IsMousePosValid()) ImGui::Text("Mouse: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
  }
  ImGui::End();
}

void Explorer::ImGuiLayer::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(ImGuiLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(ImGuiLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(ImGuiLayer::onMouseMove));
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(ImGuiLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(ImGuiLayer::onKeyReleased));
}

bool Explorer::ImGuiLayer::onMouseButtonPressed(MouseButtonPressedEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), true);
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onMouseButtonReleased(MouseButtonReleasedEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), false);
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onMouseMove(MouseMoveEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMousePosEvent(event.getX(), event.getY());
  return io.WantCaptureMouse;
}

bool Explorer::ImGuiLayer::onKeyPressed(KeyPressedEvent& event) {
  // Not implemented yet
  return false;
}

bool Explorer::ImGuiLayer::onKeyReleased(KeyReleasedEvent& event) {
  // Not implemented yet
  return false;
}

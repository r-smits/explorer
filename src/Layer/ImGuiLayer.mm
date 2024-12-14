#include <DB/Repository.hpp>
#include <Layer/ImGuiLayer.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <Renderer/Renderer.h>
#include <View/ViewAdapter.hpp>
#include <View/ViewExtender.h>
#include <imgui.h>
#include <imgui_impl_metal.h>

EXP::ImGuiLayer::ImGuiLayer(MTK::View* view, EXP::AppProperties* config)
    : Layer(view->device(), config, "ImGuiLayer"), queue(device->newCommandQueue()) {
  this->onAttach(view);
}
EXP::ImGuiLayer::~ImGuiLayer() {}

// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void EXP::ImGuiLayer::onAttach(MTK::View* view) {
  DEBUG("Initializing ImGui ...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init((__bridge id<MTLDevice>)this->device);
}

void EXP::ImGuiLayer::onDetach() {
  ImGui_ImplMetal_Shutdown();
  ImGui::DestroyContext();
}

void EXP::ImGuiLayer::onUpdate(MTK::View* pView, MTL::RenderCommandEncoder* encoder) {

  ImGuiIO& io = ImGui::GetIO();
  CGRect frame = EXP::ViewAdapter::bounds();

  io.DisplaySize.x = frame.size.width;
  io.DisplaySize.y = frame.size.height;
  io.DisplayFramebufferScale =
      ImVec2(NSScreen.mainScreen.backingScaleFactor, NSScreen.mainScreen.backingScaleFactor);

  ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor*)pView->currentRenderPassDescriptor());
  ImGui::NewFrame();

  static bool showDemo = true;
  this->showDebugWindow(&showDemo);
  ImGui::Render();

  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplMetal_RenderDrawData(
      drawData, (__bridge id<MTLCommandBuffer>)queue->commandBuffer(), (__bridge id<MTLRenderCommandEncoder>)encoder
  );

}

void EXP::ImGuiLayer::showDebugWindow(bool* open) {

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

void EXP::ImGuiLayer::onEvent(Event& event) {
  EventDispatcher dispatcher = EventDispatcher(event);
  dispatcher.dispatch<MouseButtonReleasedEvent>(BIND_EVENT(ImGuiLayer::onMouseButtonReleased));
  dispatcher.dispatch<MouseButtonPressedEvent>(BIND_EVENT(ImGuiLayer::onMouseButtonPressed));
  dispatcher.dispatch<MouseMoveEvent>(BIND_EVENT(ImGuiLayer::onMouseMove));
  dispatcher.dispatch<KeyPressedEvent>(BIND_EVENT(ImGuiLayer::onKeyPressed));
  dispatcher.dispatch<KeyReleasedEvent>(BIND_EVENT(ImGuiLayer::onKeyReleased));
}

bool EXP::ImGuiLayer::onMouseButtonPressed(MouseButtonPressedEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), true);
  return io.WantCaptureMouse;
}

bool EXP::ImGuiLayer::onMouseButtonReleased(MouseButtonReleasedEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMouseButtonEvent(event.getMouseButton(), false);
  return io.WantCaptureMouse;
}

bool EXP::ImGuiLayer::onMouseMove(MouseMoveEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
  io.AddMousePosEvent(event.getX(), event.getY());
  return io.WantCaptureMouse;
}

bool EXP::ImGuiLayer::onKeyPressed(KeyPressedEvent& event) {
  // Not implemented yet
  return false;
}

bool EXP::ImGuiLayer::onKeyReleased(KeyReleasedEvent& event) {
  // Not implemented yet
  return false;
}

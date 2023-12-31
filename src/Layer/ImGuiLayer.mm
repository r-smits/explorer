#include <Layer/ImGuiLayer.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <View/ViewExtender.h>
#include <imgui.h>
#include <imgui_impl_metal.h>
#include <imgui_impl_osx.h>

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
  ImGui_ImplOSX_Init((__bridge MTKView *)view);
}

void Explorer::ImGuiLayer::onDetach() {
  ImGui_ImplMetal_Shutdown();
  ImGui_ImplOSX_Shutdown();
  ImGui::DestroyContext();
}

void Explorer::ImGuiLayer::onUpdate(MTK::View *pView,
                                    MTL::RenderCommandEncoder *encoder) {

  // Answer: Yes, you only need the view and the encoder. That's it.
  // So let's make that the key -- passing the view and encoder.

  MTL::CommandBuffer *buffer = this->queue->commandBuffer();
  MTL::RenderPassDescriptor *descriptor = pView->currentRenderPassDescriptor();

  // MTL::CommandEncoder* encoder = buffer->renderCommandEncoder(descriptor);
  MTKView *view = (__bridge MTKView *)pView;
  ImGuiIO &io = ImGui::GetIO();

  io.DisplaySize.x = view.bounds.size.width;
  io.DisplaySize.y = view.bounds.size.height;
  CGFloat scale = view.window.screen.backingScaleFactor
                      ?: NSScreen.mainScreen.backingScaleFactor;
  io.DisplayFramebufferScale = ImVec2(scale, scale);

  ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor *)descriptor);
  ImGui_ImplOSX_NewFrame(view);
  ImGui::NewFrame();

  static bool showDemo = true;
  ImGui::ShowDemoWindow(&showDemo);
  ImGui::Render();

  ImDrawData *drawData = ImGui::GetDrawData();
  ImGui_ImplMetal_RenderDrawData(drawData,
                                 (__bridge id<MTLCommandBuffer>)buffer,
                                 (__bridge id<MTLRenderCommandEncoder>)encoder);

  // encoder->endEncoding();
}

void Explorer::ImGuiLayer::onEvent(Event &event) {}

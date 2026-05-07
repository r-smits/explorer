


#include <Layer/ImGuiAPI.h>
#include <imgui.h>
#include <imgui_impl_metal.h>
#include <View/ViewAdapter.hpp>


// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void initialize_imgui(MTL::Device* device) {
	IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init(device);
	DEBUG("Initialized ImGui");
}


void deallocate_imgui() {
  ImGui_ImplMetal_Shutdown();
  ImGui::DestroyContext();
}


void imgui_on_update(
	MTK::View* view, 
	MTL::CommandBuffer* command_buffer,
	MTL::RenderPassDescriptor* render_pass_descriptor,
	MTL::RenderCommandEncoder* render_encoder,
	MTL::Texture* output_texture
) {
		DEBUG("IMGUI on update triggered");
    ImGuiIO& io = ImGui::GetIO();
    CGRect frame = EXP::ViewAdapter::bounds();
    io.DisplaySize.x = frame.size.width;
    io.DisplaySize.y = frame.size.height;
		CGFloat scale = frame.size.width / view->drawableSize().width;
		io.DisplayFramebufferScale = ImVec2((float)scale, (float)scale);
		
		DEBUG("NewFrame fn here");
		ImGui_ImplMetal_NewFrame(render_pass_descriptor);
    ImGui::NewFrame();

    // --- Fullscreen dockspace host window ---
    ImGuiWindowFlags host_flags = \
				ImGuiWindowFlags_NoTitleBar							| 
				ImGuiWindowFlags_NoCollapse							|
        ImGuiWindowFlags_NoResize								| 
				ImGuiWindowFlags_NoMove									|
        ImGuiWindowFlags_NoBringToFrontOnFocus	| 
				ImGuiWindowFlags_NoNavFocus							|
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpace", nullptr, host_flags);
    ImGui::PopStyleVar();

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // --- Raytracer viewport panel ---
    ImGui::Begin("Viewport");
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)(void*)output_texture, size);
    ImGui::End();

    // --- Settings panel ---
    ImGui::Begin("Settings");
    ImGui::Text("Scene");
    ImGui::Separator();
    // add your actual settings here, e.g.:
    // ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f);
    ImGui::End();

    ImGui::Render();

    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, render_encoder);
}


void show_imgui_debug_window(bool* open) {

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


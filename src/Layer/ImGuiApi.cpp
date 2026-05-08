


#include <Layer/ImGuiAPI.h>
#include <imgui.h>
#include <imgui_impl_metal.h>
#include <View/ViewAdapter.hpp>
#include <imgui_internal.h>
#include <Events/IOState.h>
#include <imgui_impl_osx.h>


static bool initialized = false;


// Set up IMGUI
// https://github.com/ocornut/imgui/blob/master/examples/example_apple_metal/main.mm
void initialize_imgui(MTK::View* view) {
	IMGUI_CHECKVERSION();
  ImGui::CreateContext();
	ImGui::LoadIniSettingsFromMemory("", 0);
  ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigDockingNoSplit = true;
	io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init(view->device());
	ImGui_ImplOSX_Init(view);
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
    ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;

		// mouse position
		//auto [x, y] = EXP::IO::getMouse();
		//io.AddMousePosEvent(x, y);
		//(EXP::IO::isPressed(MOUSE)) ? io.AddMouseButtonEvent(0, true) : io.AddMouseButtonEvent(0, false);

    CGRect frame = EXP::ViewAdapter::bounds();
		io.DisplaySize.x = frame.size.width * 2;
		io.DisplaySize.y = frame.size.height * 2;
		float scale = (float)view->drawableSize().width / (float)frame.size.width;
		io.DisplayFramebufferScale = ImVec2(scale, scale);
		ImGui_ImplMetal_NewFrame(render_pass_descriptor);
		ImGui_ImplOSX_NewFrame(view);
    ImGui::NewFrame();

    ImGuiWindowFlags host_flags = \
				ImGuiWindowFlags_NoTitleBar							| 
				ImGuiWindowFlags_NoCollapse							|
        ImGuiWindowFlags_NoResize								| 
				//ImGuiWindowFlags_NoMove									|
        ImGuiWindowFlags_NoBringToFrontOnFocus	| 
				ImGuiWindowFlags_NoNavFocus							|
        ImGuiWindowFlags_NoBackground;

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(
			{static_cast<float>(frame.size.width), static_cast<float>(frame.size.height)}, 
			ImGuiCond_FirstUseEver
		);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, .0f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, .0f});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .0f);
		ImGui::Begin("DockSpace", nullptr, host_flags);
    
		ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
		if (!initialized) {
			DEBUG("Initializing ImGui Docking ...");
			initialized = true;
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id);
			ImGui::DockBuilderSetNodeSize(
				dockspace_id, 
				{static_cast<float>(frame.size.width), static_cast<float>(frame.size.height)}
			);

			ImGuiID dock_right;
			ImGuiID dock_main = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.75f, nullptr, &dock_right);

			ImGui::DockBuilderDockWindow("Viewport", dock_main);
			ImGui::DockBuilderDockWindow("Settings", dock_right);
			ImGui::DockBuilderFinish(dockspace_id);
		}
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();

    ImGui::DockSpace(dockspace_id, {.0f, .0f}, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingSplit);
		ImGui::End();

    // --- Raytracer viewport panel ---
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, .0f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, 4.f});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .1f);

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImVec2 size = ImGui::GetContentRegionAvail();
		    ImGui::Image((ImTextureID)(void*)output_texture, {size.x * 2, size.y * 2});
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
    ImGui::End();

    // --- Settings panel ---
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::Begin("Settings");
    ImGui::Text("Scene");
    ImGui::Separator();
    // add your actual settings here, e.g.:
    // ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
    ImGui::End();

    ImGui::Render();

    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, render_encoder);
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
}


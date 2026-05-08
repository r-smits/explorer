


#include <Layer/ImGuiAPI.h>
#include <imgui.h>
#include <imgui_impl_metal.h>
#include <View/ViewAdapter.hpp>
#include <imgui_internal.h>
#include <Events/IOState.h>
#include <imgui_impl_osx.h>
#include <Model/ResourceManager.h>


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
	io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplMetal_Init(view->device());
	ImGui_ImplOSX_Init(view);


	DEBUG("Initialized ImGui");
}


void deallocate_imgui() {
  ImGui_ImplMetal_Shutdown();
	ImGui_ImplOSX_Shutdown();
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
				ImGuiWindowFlags_NoMove									|
        ImGuiWindowFlags_NoBringToFrontOnFocus  | 
				ImGuiWindowFlags_NoNavFocus							|
        ImGuiWindowFlags_NoBackground						| 
				ImGuiWindowFlags_NoNavInputs
				;
	
		ImGuiWindowFlags node_flags = \
			ImGuiWindowFlags_NoScrollbar							| 
			ImGuiWindowFlags_NoBringToFrontOnFocus		| 
			ImGuiWindowFlags_NoResize									|
			ImGuiWindowFlags_NoCollapse								|
			ImGuiWindowFlags_NoNavFocus								|
			ImGuiWindowFlags_NoScrollWithMouse				|
			ImGuiWindowFlags_NoNav										|
			ImGuiWindowFlags_NoNavInputs							|
			ImGuiWindowFlags_AlwaysUseWindowPadding		|
			ImGuiDockNodeFlags_NoTabBar;


    const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImVec2 vec_size {static_cast<float>(frame.size.width), static_cast<float>(frame.size.height)};
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vec_size, ImGuiCond_Always);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, 4.f});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .0f);
		ImGui::Begin("DockSpace", nullptr, host_flags);
    
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		if (!initialized) {
			DEBUG("Initializing ImGui Docking ...");
			initialized = true;
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(
				dockspace_id, 
				ImGuiDockNodeFlags_NoWindowMenuButton
			);

			ImGui::DockBuilderSetNodeSize(dockspace_id, vec_size);

			ImGuiID settings_node_id;
			ImGuiID browser_node_id;
			ImGuiID left_node_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.75f, nullptr, &settings_node_id);
			ImGuiID raytracer_node_id = ImGui::DockBuilderSplitNode(left_node_id, ImGuiDir_Up, .9f, nullptr, &browser_node_id);

			ImGui::DockBuilderDockWindow("Raytracer", raytracer_node_id);
			ImGui::DockBuilderDockWindow("Settings", settings_node_id);
			ImGui::DockBuilderDockWindow("Browser", browser_node_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}
		ImGui::DockSpace(
			dockspace_id, 
			vec_size, 
			ImGuiDockNodeFlags_PassthruCentralNode | 
			ImGuiDockNodeFlags_NoDockingSplit			 
			//|
			//ImGuiDockNodeFlags_NoResize
		);
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
		ImGui::End();

    // --- Raytracer viewport panel ---
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, 0.0f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, 4.f});
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .1f);
    ImGui::Begin("Raytracer", nullptr, node_flags);
    ImVec2 size = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)(void*)output_texture, {size.x * 2, size.y * 2});
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
    ImGui::End();

    // --- Settings panel ---
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, 0.0f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, 4.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .1f);

		ImGui::Begin("Settings", nullptr, node_flags);
    ImGui::Text("Scene");
    ImGui::Separator();
		
		std::vector<EXP::Model*> models = EXP::SCENE::getModels();
		for (int i = 0; i < models.size(); i += 1) {
			ImGui::SliderFloat(
				(models[i]->name + "_scale_" + std::to_string(i)).c_str(), 
				&models[i]->m_scalar, 
				.01f, 1.5f
			);
			models[i]->scale(models[i]->m_scalar);
		}
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
    ImGui::End();

		// --- Browser panel ---
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {.0f, 0.0f});
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {.0f, 4.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, .1f);

		ImGui::Begin("Browser", nullptr, node_flags);
    ImGui::Text("Meshes");
    ImGui::Separator();
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
    ImGui::End();
		
		if (ImGui::IsKeyDown(ImGuiKey_T)) {
			EXP::SCENE::getCamera()->setMoved(true);
			for (EXP::Model *model : EXP::SCENE::getModels()) {
				model->rotate(EXP::MATH::yRotation(-1.0f));
			}
		}


    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, render_encoder);
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
}


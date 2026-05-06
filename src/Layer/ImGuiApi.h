#pragma once
#include <pch.h>


void initialize_imgui(MTL::Device* view); 
void deallocate_imgui();
void imgui_on_update(
	MTK::View* view, 
	MTL::CommandBuffer* command_buffer,
	MTL::RenderPassDescriptor* render_pass_descriptor,
	MTL::RenderCommandEncoder* render_encoder,
	MTL::Texture* output_texture
);
void show_imgui_debug_window(bool* open);


#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"
#import "../src/Shaders/RayUtils.h"


[[kernel]]
void temporal_reuse(
	uint2 tid										[[ thread_position_in_grid	]], 
	texture2d<float, access::write> buffer			[[ texture(0) 				]],
	instance_acceleration_structure structure		[[ buffer(1)				]],
	constant Scene* scene							[[ buffer(2)				]]
) {

	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Retrieving initial colors and values	//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//

	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(float4(.0f), tid);
		return;
	}
	
	thread uint32_t seed = tid.x * 1619 + tid.y * 31337 + scene->vcamera->frameCount * 719393;
	thread ray r = project_ray(scene->vcamera, tid);
	thread Hit hit = color_ray(r, structure, scene, seed);
	if (!hit.did_hit) hit = intersect_plane(r, -0.2f);
	if (!hit.did_hit || hit.is_light) {
		buffer.write(hit.color, tid);
		return;
	}

	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Global Illumination						//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//

	thread float4 curr_reservoir = float4(.0f);
	thread float l_dot_n = float(0.0f);
	thread int light_count = scene->lights[0].vertexCount;
	thread float uniform_pdf_sample = 1.0f / light_count;
	thread float3 color_over_pi = hit.color.xyz / M_PI_F;
	thread float complex_pdf_sample = float(0.0f);
	thread float prev_p_hat_weight = float(0.0f);
	thread float3 luminance = float3(0.2126f, 0.7152f, 0.0722f);

	auto p_hat = [&](LightSample ls) -> float {
    	return dot(color_over_pi * ls.color.xyz * ls.l_dot_n / ls.distance, luminance) / uniform_pdf_sample;
	};

	for (int i = 0; i < min(light_count, 32); i += 1) {
		// TODO: create a new abstraction with: (a) vertex attributes, (b) vertices, (c) total vertex count
		// TODO: generate a single number to randomly choose a light and position to sample.
		int light_index = min(int(rand(seed) * light_count), light_count-1);

		// Update reservoir as a float4
		thread LightSample lsample = sample_light(scene, light_index, r.origin, hit.normal);
		update_reservoir(curr_reservoir, light_index, p_hat(lsample), seed); 
	}
	
	// Retrieve final selected weight
	thread LightSample lsample = sample_light(scene, curr_reservoir.y, r.origin, hit.normal);
	complex_pdf_sample = p_hat(lsample);
	curr_reservoir.w = (curr_reservoir.x / curr_reservoir.z) / max(complex_pdf_sample, 1e-4f);

	// Shadow ray for current reservoir
	thread bool visible = shadow_ray(r, structure, lsample.direction, lsample.world_pos);
	curr_reservoir.w *= visible;

	// Add current reservoir to combined reservoir 
	float4 combined_reservoir = float4(.0f);
	update_reservoir(combined_reservoir, curr_reservoir.y, complex_pdf_sample * curr_reservoir.w * curr_reservoir.z, seed);
	
	// Add previous reservoir to combined reservoir
	uint2 prev_frame_tid = reproject_ray(r, scene->prev_vcamera);
	float4 prev_reservoir = scene->textreadwrite[RestirIdx::prev_frame].value.read(prev_frame_tid);
	lsample = sample_light(scene, prev_reservoir.y, r.origin, hit.normal);
	prev_reservoir.z = min(20.f * curr_reservoir.z, prev_reservoir.z);
	update_reservoir(combined_reservoir, prev_reservoir.y, p_hat(lsample) * prev_reservoir.w * prev_reservoir.z, seed);
	
	// Set sample size and adjusted weight of combined reservoir
	combined_reservoir.z = curr_reservoir.z + prev_reservoir.z;
	lsample = sample_light(scene, combined_reservoir.y, r.origin, hit.normal);
	combined_reservoir.w = (combined_reservoir.x / combined_reservoir.z) / max(p_hat(lsample), 1e-4f);
	
	// Shadow ray for combined reservoir	
	visible = shadow_ray(r, structure, lsample.direction, lsample.world_pos);
	scene->textreadwrite[RestirIdx::prev_frame].value.write(combined_reservoir, tid);

	float4 shade_color = 
		float4(color_over_pi * lsample.color.xyz * lsample.l_dot_n / lsample.distance * visible * combined_reservoir.w, 1.f);
	
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Indirect Illumination					//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	
	r.direction = rand_hemisphere(seed, hit.normal);
	float sample_probability = 1.0f / (2.0f * M_PI_F);
	float n_dot_l = lambertian(r.direction, hit.normal);
	
	float4 indirect_color = transport_ray(r, structure, scene, tid, 1, seed);
	indirect_color = float4(n_dot_l * indirect_color.rgb * color_over_pi / sample_probability, 1.f);
	float4 current = indirect_color + shade_color;

	// Clamp fireflies
	float luma = dot(current.rgb, float3(0.2126f, 0.7152f, 0.0722f));
	float maxLuma = 1.f;
	if (luma > maxLuma) {
		current.rgb *= maxLuma / luma;
	}

	// Accumulation
	texture2d<float, access::read_write> accum = scene->textreadwrite[RestirIdx::accumulation].value;
	if (scene->vcamera->moved) {
		accum.write(current, tid);
		buffer.write(current, tid);
	} else {
		float4 history = accum.read(tid);
		float alpha = 0.1f;
		float4 accumulated = mix(history, current, alpha);
		accum.write(accumulated, tid);
		buffer.write(accumulated, tid);	
	}
}


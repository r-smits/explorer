#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"
#import "../src/Shaders/RayUtils.h"

void intersect_and_do_nothing(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread uint32_t& seed,
	thread float4& contribution,
	thread bool& bounce_continue
) {
	
	float3 normal = float3(.0f);
	float3 direction = float3(.0f);
	float3 vec_light_origin = float3(.0f);
	float3 vec_to_light = float3(.0f);
	float4 light_color = float4(.0f);
	float distance_to_light = .0f;
	bool visible = true;
	bounce_continue = false;

	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> result;
	result = intersector.intersect(r, structure, 0xFF);

	if (result.type == intersection_type::triangle) {
		bounce_continue = true;
		
		float2 bary_2d = result.triangle_barycentric_coord;
		float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

		const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) result.primitive_data;

		// Calculate variables needed to solve the rendering equation
		normal = (prim->normal[0] * bary_3d.x) + (prim->normal[1] * bary_3d.y) + (prim->normal[2] * bary_3d.z);
		normal = normalize(result.object_to_world_transform * float4(normal, 0.0f));
		seed = gid.x * (result.primitive_id * bary_3d.z * 1) + gid.y * (result.primitive_id * bary_2d.y);
			
		// We assume for now that the surface is perfectly reflective.
		// You can chance this for materials and so forth
		// Called: BSDF: bi-directional scattering distribution function.
		r.origin = r.origin + r.direction * result.distance;
		float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .0);
		r.direction = reflect(r.direction, jittered_normal);
		float wi_dot_n = lambertian(r.direction, normal);
			
		// Calculate all color contributions; use texture, emission if there is one
		float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
		
		float4 sampled_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord); 
		// float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
		float4 wo_color = sampled_color + prim->color[0];	
			
		contribution = float4(txcoord, .0f, 1.0f);
		// contribution *= 1 * wo_color;
		//contribution *= 1 * wi_dot_n * wo_color * 1 * 1;


		/**
		// Sampling light and assessing shadow ray visibility
		float light_index = float(min(int(rand(seed) * scene->lights[0].vertexCount), scene->lights[0].vertexCount-1));
		sample_light(scene, light_index, r.origin, vec_light_origin, vec_to_light, light_color, distance_to_light);
		float l_dot_n = min(1.f, lambertian(vec_to_light, normal) + prim->flags[1]); 
	
		visible = shadow_ray(r, structure, vec_to_light, vec_light_origin);
		contribution *= visible * wi_dot_n * wo_color * light_color * l_dot_n;
	
		bounce_continue = !prim->flags[1];
		**/	
	}
}

void shade_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread uint32_t& seed,
	thread float4& contribution,
	thread bool& bounce_continue
) {
	
	float3 normal = float3(.0f);
	float3 direction = float3(.0f);
	float3 vec_light_origin = float3(.0f);
	float3 vec_to_light = float3(.0f);
	float4 light_color = float4(.0f);
	float distance_to_light = .0f;
	bool visible = true;

	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> result;
	result = intersector.intersect(r, structure, 0xFF);

	// If our ray does not hit, terminate early.
	if (result.type == intersection_type::none) {
		bounce_continue = false;
	} else {
		
	float2 bary_2d = result.triangle_barycentric_coord;
	float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

	const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) result.primitive_data;

	// Calculate variables needed to solve the rendering equation
	normal = (prim->normal[0] * bary_3d.x) + (prim->normal[1] * bary_3d.y) + (prim->normal[2] * bary_3d.z);
	normal = normalize(result.object_to_world_transform * float4(normal, 0.0f));
	seed = gid.x * (result.primitive_id * bary_3d.z * 1) + gid.y * (result.primitive_id * bary_2d.y);
			
	// We assume for now that the surface is perfectly reflective.
	// You can chance this for materials and so forth
	// Called: BSDF: bi-directional scattering distribution function.
	r.origin = r.origin + r.direction * result.distance;
	float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .0);
	r.direction = reflect(r.direction, jittered_normal);
	float wi_dot_n = lambertian(r.direction, normal);
			
	// Calculate all color contributions; use texture, emission if there is one
	float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
	float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
		
	// Sampling light and assessing shadow ray visibility
	float light_index = float(min(int(rand(seed) * scene->lights[0].vertexCount), scene->lights[0].vertexCount-1));
	sample_light(scene, light_index, r.origin, vec_light_origin, vec_to_light, light_color, distance_to_light);
	float l_dot_n = min(1.f, lambertian(vec_to_light, normal) + prim->flags[1]); 
	
	visible = shadow_ray(r, structure, vec_to_light, vec_light_origin);
	contribution *= visible * wi_dot_n * wo_color * light_color * l_dot_n;
	
	bounce_continue = !prim->flags[1];
	}
}


float4 transport_ray(
	thread ray& r,
	instance_acceleration_structure structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread uint32_t& seed
) {
	
	// Contribution is set to color because it will continue from previous light transport.
	float4 contribution = float(1.0f);
	float4 sky_color = float4(.3f, .4f, .5f, 1.0f);
	float4 color = float4(.0f);
	bool bounce_continue = true;
	
	for (int i = 1; i <= bounces; i += 1) {
		shade_ray(r, structure, scene, gid, bounces, seed, contribution, bounce_continue); 
	}
		
	color += contribution * sky_color;
	return color;
}


[[kernel]]
void temporal_reuse(
	uint2 tid																							[[ thread_position_in_grid	]], 
	texture2d<float, access::write> buffer								[[ texture(0) ]],
	instance_acceleration_structure structure							[[ buffer(1)	]],
	constant Scene* scene																	[[ buffer(2)	]]
) {
	
	float4 curr_reservoir = float4(.0f);
	float4 color = float4(.0f);
	float3 vec_origin = float3(.0f);
	float3 vec_normal = float3(.0f);
	thread uint32_t seed = (1 + tid.x) * (tid.y - tid.x) + (1 + tid.y) * (tid.x + tid.y); 
	bool light = false;
	
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Retrieving initial colors and values	//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	

	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, tid);
		return;
	}

	ray r;
	build_ray(r, scene->vcamera, tid);
	if (!color_ray(r, structure, scene, tid, color, vec_origin, vec_normal, seed, light) || light) {
		buffer.write(color, tid);
		return;
	}
	
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Global Illumination										//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//

	float distance_to_light = float(0.0f);
	float l_dot_n = float(0.0f);
	float light_index = float(.0f);
	float p_hat_weight = float(0.0f);
	float prev_p_hat_weight = float(0.0f);
	float3 vec_ray_direction = float3(.0f);
	float3 vec_to_light = float3(.0f);
	float4 vec_light_col = float4(.0f);
	float3 vec_world_light_pos = float3(0.0f);
	
	for (int i = 0; i < min(int(scene->lightsCount), 32); i += 1) {
		
		// Generating light indices
		// TODO: create a new abstraction with: (a) vertex attributes, (b) vertices, (c) total vertex count
		// TODO: generate a single number to randomly choose a light and position to sample.
		// int light_sample_index = min(int(rand(seed) * scene->lightsCount), int(scene->lightsCount-1));
		int vertex_count = scene->lights[0].vertexCount;
		float light_index = float(min(int(rand(seed) * scene->lights[0].vertexCount), scene->lights[0].vertexCount-1));
	
		// Update reservoir as a float4
		sample_light(scene, light_index, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
		l_dot_n = lambertian(vec_to_light, vec_normal); 
		p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
		update_reservoir(curr_reservoir, light_index, p_hat_weight, seed); 
	}
	
	// Retrieve final selected weight
	sample_light(scene, curr_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = lambertian(vec_to_light, vec_normal); 
	p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	curr_reservoir.w = (1.f / max(p_hat_weight, 0.0001f)) * (curr_reservoir.x / max(curr_reservoir.z, 0.0001f));

	// Shadow ray for current reservoir
	bool visible = shadow_ray(r, structure, vec_to_light, vec_world_light_pos);
	curr_reservoir.w *= visible;

	// Add current reservoir to combined reservoir 
	float4 combined_reservoir = float4(.0f);
	update_reservoir(combined_reservoir, curr_reservoir.y, p_hat_weight * curr_reservoir.w * curr_reservoir.z, seed);
	
	// Add previous reservoir to combined reservoir
	float4 prev_reservoir = scene->textreadwrite[RestirIdx::prev_frame].value.read(tid);
	sample_light(scene, prev_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = lambertian(vec_to_light, vec_normal); 
	prev_p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	prev_reservoir.z = min(20.f * curr_reservoir.z, prev_reservoir.z);
	update_reservoir(combined_reservoir, prev_reservoir.y, prev_p_hat_weight * prev_reservoir.w * prev_reservoir.z, seed);
	
	// Set sample size and adjusted weight of combined reservoir
	combined_reservoir.z = curr_reservoir.z + prev_reservoir.z;
	sample_light(scene, combined_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = lambertian(vec_to_light, vec_normal); 
	p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	combined_reservoir.w = (1.f / max(p_hat_weight, 0.0001f)) * (combined_reservoir.x / max(combined_reservoir.z, 0.0001f));
	
	// Shadow ray for combined reservoir	
	visible = shadow_ray(r, structure, vec_to_light, vec_world_light_pos);
	scene->textreadwrite[RestirIdx::prev_frame].value.write(combined_reservoir, tid);
	float4 shade_color = float4(visible * combined_reservoir.w * l_dot_n * vec_light_col.xyz * color.rgb / M_PI_F, 1.f);
	
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Indirect Illumination									//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	
	//r.direction = r.direction + rand_hemisphere(seed, vec_normal) * .05;
	float sample_probability = 1.0f / (2.0f * M_PI_F);
	
	float4 indirect_color = transport_ray(r, structure, scene, tid, 1, seed);
	indirect_color = float4(l_dot_n * indirect_color.rgb * color.rgb / M_PI_F / sample_probability, 1.f);
	buffer.write(indirect_color + shade_color, tid);
}



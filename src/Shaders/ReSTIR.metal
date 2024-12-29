#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"
#import "../src/Shaders/RayUtils.h"



float4 transport_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread uint32_t& seed
) {
	
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection;
	
	// Contribution is set to color because it will continue from previous light transport.
	float4 contribution = float(1.0f);
	float4 sky_color = float4(.2f, .3f, .4f, 1.0f);
	float4 color = float4(.0f);
	float3 normal = float3(.0f);
	
	float3 direction = float3(.0f);
	float3 vec_light_origin = float3(.0f);
	float3 vec_to_light = float3(.0f);
	float4 light_color = float4(.0f);
	float distance_to_light = .0f;
	bool visible = true;

	for (int i = 1; i <= bounces; i++) {
		// Verify if ray has intersected the geometry
		intersection = intersector.intersect(r, structure, 0xFF);
		
		// If our ray does not hit, terminate early.
		if (intersection.type == intersection_type::none) { 
			color += contribution * visible * sky_color;
			return color;
		}
		
		float2 bary_2d = intersection.triangle_barycentric_coord;
		float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

		const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;

		// Calculate variables needed to solve the rendering equation
		normal = (prim->normal[0] * bary_3d.x) + (prim->normal[1] * bary_3d.y) + (prim->normal[2] * bary_3d.z);
		normal = normalize(intersection.object_to_world_transform * float4(normal, 0.0f));
		seed = gid.x * (intersection.primitive_id * bary_3d.z * i) + gid.y * (intersection.primitive_id * bary_2d.y);
			
		// We assume for now that the surface is perfectly reflective.
		// You can chance this for materials and so forth
		// Called: BSDF: bi-directional scattering distribution function.
		r.origin = r.origin + r.direction * intersection.distance;
		float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .0);
		r.direction = reflect(r.direction, jittered_normal);
		float wi_dot_n = lambertian(r.direction, normal);
			
		// Calculate all color contributions; use texture, emission if there is one
		float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
		float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
		
		// Sampling light and assessing shadow ray visibility
		float light_index = float(min(int(rand(seed) * scene->lights[0].vertexCount), scene->lights[0].vertexCount-1));
		sample_light(scene, light_index, r.origin, vec_light_origin, vec_to_light, light_color, distance_to_light);
		
		direction = r.direction;
		r.direction = vec_to_light;
		visible = shadow_ray(r, structure, vec_light_origin);
		r.direction = direction;

		contribution *= visible * wo_color * light_color * wi_dot_n / M_PI_F;
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
	thread uint32_t seed = tid.x + tid.y; 
	bool light = false;
	
	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, tid);
		return;
	}

	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Retrieving initial colors and values	//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	
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
		l_dot_n = saturate(dot(vec_normal, vec_to_light));
		p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
		update_reservoir(curr_reservoir, light_index, p_hat_weight, seed); 
	}
	
	// Retrieve final selected weight
	sample_light(scene, curr_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = saturate(dot(vec_normal, vec_to_light));
	p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	curr_reservoir.w = (1.f / max(p_hat_weight, 0.0001f)) * (curr_reservoir.x / max(curr_reservoir.z, 0.0001f));
	
	vec_ray_direction = r.direction;
	r.direction = vec_to_light;
	bool visible = shadow_ray(r, structure, vec_world_light_pos);
	curr_reservoir.w *= visible;
	r.direction = vec_ray_direction;

	// Add current reservoir to combined reservoir 
	float4 combined_reservoir = float4(.0f);
	update_reservoir(combined_reservoir, curr_reservoir.y, p_hat_weight * curr_reservoir.w * curr_reservoir.z, seed);
	
	// Add previous reservoir to combined reservoir
	float4 prev_reservoir = scene->textreadwrite[RestirIdx::prev_frame].value.read(tid);
	sample_light(scene, prev_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = saturate(dot(vec_normal, vec_to_light));
	prev_p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	prev_reservoir.z = min(20.f * curr_reservoir.z, prev_reservoir.z);
	update_reservoir(combined_reservoir, prev_reservoir.y, prev_p_hat_weight * prev_reservoir.w * prev_reservoir.z, seed);
	
	// Set sample size and adjusted weight of combined reservoir
	combined_reservoir.z = curr_reservoir.z + prev_reservoir.z;
	sample_light(scene, combined_reservoir.y, r.origin, vec_world_light_pos, vec_to_light, vec_light_col, distance_to_light);
	l_dot_n = saturate(dot(vec_normal, vec_to_light));
	p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / distance_to_light);
	combined_reservoir.w = (1.f / max(p_hat_weight, 0.0001f)) * (combined_reservoir.x / max(combined_reservoir.z, 0.0001f));

	scene->textreadwrite[RestirIdx::prev_frame].value.write(combined_reservoir, tid);
	float4 shade_color = float4(visible * combined_reservoir.w * l_dot_n * vec_light_col.xyz * color.xyz / M_PI_F, 1.f);

	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	//	Indirect Illumination									//
	//	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	//
	
	r.direction = rand_hemisphere(seed, vec_normal);
	float n_dot_l = saturate(dot(vec_normal, r.direction));
	float sample_probability = 1.0f / (2.0f * M_PI_F);
	
	float4 indirect_color = transport_ray(r, structure, scene, tid, 2, seed);
	indirect_color = float4((n_dot_l * indirect_color.xyz * color.xyz / M_PI_F / sample_probability), 1.f);

	buffer.write(shade_color + indirect_color, tid);
}



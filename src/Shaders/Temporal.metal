#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"


bool shadow_ray(
	ray r,
	instance_acceleration_structure structure,
	float3 vec_light_origin
) {
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection;
	intersection = intersector.intersect(r, structure, 0xFF);

	if (intersection.type == intersection_type::triangle) {
		const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;
		float3 origin = r.origin + r.direction * intersection.distance;
		if (prim->flags[1] || abs(distance(vec_light_origin, origin)) < 0.1) return true;
	}
	return false;
}


bool transport_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread float4& color,
	thread float3& normal,
	thread uint32_t& seed,
	thread bool& light
) {
	
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection;

	// Contribution is set to 1 and will decrease over time, with the amount of bounces
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 sky_color = float4(.4f, .5f, .6f, 1.0f);

	for (int i = 1; i <= bounces; i++) {
		// Verify if ray has intersected the geometry
		intersection = intersector.intersect(r, structure, 0xFF);

		// If our ray does not hit, terminate early.
		if (intersection.type == intersection_type::none) { 
			color += contribution * sky_color;
			return (i != 1);
		} 
		if (intersection.type == intersection_type::triangle) {

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
			float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .2);
			r.direction = reflect(r.direction, jittered_normal);
			float wi_dot_n = lambertian(r.direction, normal);
			
			// Calculate all color contributions; use texture, emission if there is one
			float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
			float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
			
			contribution *= wo_color * wi_dot_n;
			light = prim->flags[1]; 
		}
	}
	color += contribution * sky_color;
	return true;
}


void sample_light(
	constant Scene* scene,
	uint32_t seed,
	float3 vec_world_hit,
	thread float& distance_to_light,
	thread float3& vec_to_light,
	thread float4& vec_light_col,
	thread float3& vec_world_light_pos,
	float light_index
) {
	// Retrieve light vertex data
	int index = int(light_index);
	vec_world_light_pos = (scene->lights[0].orientation * float4(scene->lights[0].vertices[index], 1.0f)).xyz;

	// Set variables by reference
	distance_to_light = abs(distance(vec_world_light_pos, vec_world_hit));
	vec_to_light = normalize(vec_world_light_pos - vec_world_hit);
	vec_light_col = scene->lights[0].attributes[index].color;
}


void update_reservoir(
	thread float4& reservoir, 
	float light_indices, 
	float p_hat_weight, 
	thread uint32_t& seed
) {
	reservoir.x += p_hat_weight;											// w_sum - total sum of weights
	reservoir.z += 1.0f;															// m_sum - total sum of samples
	float random = rand(seed);
	if (random <= (p_hat_weight / reservoir.x)) {
		reservoir.y = light_indices;										// sample inside of reservoir
	}
}


[[kernel]]
void temporal_reuse(
	uint2 tid																							[[ thread_position_in_grid	]], 
	texture2d<float, access::write> buffer								[[ texture(0) ]],
	instance_acceleration_structure structure							[[ buffer(1)	]],
	constant Scene* scene																	[[ buffer(2)	]]
) {
	
	float4 curr_reservoir = float4(0.0f);
	float4 color = float4(0.0f);
	float3 vec_normal = float3(0.0f);
	float3 vec_normal_2 = float3(0.0f);
	thread uint32_t seed = 0; 
	bool light = false;

	if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, tid);
		return;
	}
	
	ray r;
	build_ray(r, scene->vcamera, tid);
	if (!transport_ray(r, structure, scene, tid, 1, color, vec_normal, seed, light)) {
		buffer.write(color, tid);
		return;
	}
	
	float distance_to_light = float(0.0f);
	thread float light_indices = float(0.0f);
	float p_hat_weight = float(0.0f);
	float prev_p_hat_weight = float(0.0f);
	float3 vec_to_light = float3(0.0f);
	float4 vec_light_col = float4(0.0f);
	float3 vec_world_light_pos = float3(0.0f);
	float l_dot_n = float(0.0f);
	float light_index = float(.0f);
	
	for (int i = 0; i < min(int(scene->lightsCount), 32); i += 1) {
		
		// Generating light indices
		// TODO: create a new abstraction with: (a) vertex attributes, (b) vertices, (c) total vertex count
		// TODO: generate a single number to randomly choose a light and position to sample.
		// int light_sample_index = min(int(rand(seed) * scene->lightsCount), int(scene->lightsCount-1));
		int vertex_count = scene->lights[0].vertexCount;
		float light_index = float(min(int(rand(seed) * vertex_count), vertex_count-1));
	
		// Update reservoir as a float4
		sample_light(scene, seed, r.origin, distance_to_light, vec_to_light, vec_light_col, vec_world_light_pos, light_index);

		l_dot_n = saturate(dot(vec_normal, vec_to_light));
		p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / (distance_to_light * distance_to_light));
		update_reservoir(curr_reservoir, light_index, p_hat_weight, seed); 
	}
	
	// Retrieve final selected weight
	sample_light(scene, seed, r.origin, distance_to_light, vec_to_light, vec_light_col, vec_world_light_pos, curr_reservoir.y);
	
	l_dot_n = saturate(dot(vec_normal, vec_to_light));
	float3 shading_color = color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / (distance_to_light * distance_to_light);
	p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / (distance_to_light * distance_to_light));
	curr_reservoir.w = (1.f / max(p_hat_weight, 0.0001f)) * (curr_reservoir.x / max(curr_reservoir.z, 0.0001f));
	
	r.direction = normalize(vec_to_light);

	if (light) {
		buffer.write(color, tid);	
		return;
	}
	bool visible = shadow_ray(r, structure, vec_world_light_pos);
	curr_reservoir.w *= visible;
	
	// Add current and previous reservoir to new combined reservoir 
	float4 combined_reservoir = float4(.0f);
	update_reservoir(combined_reservoir, curr_reservoir.y, p_hat_weight * curr_reservoir.w * curr_reservoir.z, seed);
	
	float4 prev_reservoir = scene->textreadwrite[RestirIdx::prev_frame].value.read(tid);
	sample_light(scene, seed, r.origin, distance_to_light, vec_to_light, vec_light_col, vec_world_light_pos, prev_reservoir.y);
	l_dot_n = saturate(dot(vec_normal, vec_to_light));
	prev_p_hat_weight = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / (distance_to_light * distance_to_light));
	prev_reservoir.z = min(20.f * curr_reservoir.z, prev_reservoir.z);
	update_reservoir(combined_reservoir, prev_reservoir.y, prev_p_hat_weight * prev_reservoir.w * prev_reservoir.z, seed);
	
	combined_reservoir.z = curr_reservoir.z + prev_reservoir.z;
	sample_light(scene, seed, r.origin, distance_to_light, vec_to_light, vec_light_col, vec_world_light_pos, prev_reservoir.y);
	l_dot_n = saturate(dot(vec_normal, vec_to_light));

	combined_reservoir.w = length(color.xyz / M_PI_F * vec_light_col.xyz * l_dot_n / (distance_to_light * distance_to_light));
	scene->textreadwrite[RestirIdx::prev_frame].value.write(combined_reservoir, tid);

	float3 shade_color = visible * combined_reservoir.w * l_dot_n * vec_light_col.xyz * color.xyz / M_PI_F;
	buffer.write(float4(shade_color, .0f), tid);
}



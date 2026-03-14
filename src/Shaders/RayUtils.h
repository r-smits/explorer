#pragma once

#ifndef RayUtils_h
#define RayUtils_h

#if __METAL_VERSION__


void build_ray(thread ray& r, constant VCamera* vcamera, uint2 gid) {
	float2 uv = (float2(gid) / vcamera->resolution.xy) * 2.0f - 1.0f;
    uv.y *= -1.0f;

    float aspectRatio = vcamera->resolution.x / vcamera->resolution.y;
    float orthoScale = vcamera->fovScale; // repurpose as ortho half-width

    float3 right   = float3(vcamera->vecRight);
    float3 up      = float3(vcamera->vecUp);
    float3 forward = float3(vcamera->vecForward);

    // All rays parallel, origins spread across the view plane
    r.origin    = vcamera->vecOrigin 
                + uv.x * orthoScale * aspectRatio * right 
                + uv.y * orthoScale * up - forward * 5;
    r.direction = normalize(forward);
    r.min_distance = 0.1f;
    r.max_distance = FLT_MAX;
}


bool intersect_ground_plane(
    thread ray& r, 
    float plane_y,
    thread float& distance,
	thread float3& vec_normal,
	thread float4& color
) {
    // Ray-plane intersection: r.origin.y + t * r.direction.y = plane_y
    float denom = r.direction.y;
    if (abs(denom) < 1e-6f) return false;
    distance = (plane_y - r.origin.y) / denom;
    if (distance < r.min_distance || distance > r.max_distance) return false;

	r.origin = r.origin + r.direction * distance;
	vec_normal = float3(0.0f, 1.0f, 0.0f);
	
	// Grid pattern as the surface color
	float2 grid = abs(fract(r.origin.xz * 5.0f) - .5f);
	float line = min(grid.x, grid.y);
	color = mix(float4(.3f, .3f, .3f, 1.f), float4(.1f, .1f, .1f, 1.f), step(line, 0.01f));
	return true;
}


bool shadow_ray(
	thread ray& s,
	thread instance_acceleration_structure& structure,
	float3 vec_to_light,
	float3 vec_light_origin
) {
	bool result = false;
	float prev_min_distance = s.min_distance;
	float3 prev_direction = s.direction;

	s.min_distance = 0.001f;						// Set to avoid self-occlusion
	s.direction = vec_to_light;

	intersector<instancing, triangle_data> shadow_intersector;	
	shadow_intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data> shadow_intersection;
	shadow_intersection = shadow_intersector.intersect(s, structure, 0xFF);

	if (shadow_intersection.type == intersection_type::triangle) {
		// const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) shadow_intersection.primitive_data;
		float3 origin = s.origin + s.direction * shadow_intersection.distance;
		result = (abs(distance(vec_light_origin, origin)) < 0.001);
	}

	s.min_distance = prev_min_distance;
	s.direction = prev_direction;
	return result;
}


void sample_light(
	constant Scene* scene,
	int light_index,
	float3 vec_ray_world_pos,
	thread float3& vec_world_light_pos,
	thread float3& vec_to_light,
	thread float4& vec_light_col,
	thread float& distance_to_light
) {
	// Retrieve light vertex data
	vec_world_light_pos = (scene->lights[0].orientation * float4(scene->lights[0].vertices[light_index], 1.0f)).xyz;

	// Set variables by reference
	vec_to_light = normalize(vec_world_light_pos - vec_ray_world_pos);
	vec_light_col = scene->lights[0].attributes[light_index].color;
	distance_to_light = distance_squared(vec_world_light_pos, vec_ray_world_pos);
}


bool color_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	thread float4& color,
	thread float3& vec_normal,
	thread uint32_t& seed,
	thread bool& light
) {
	
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection;
	
	float4 sky_color = float4(.3f, .4f, .5f, 1.0f);
	float4 contribution = float4(1.0f, 1.0f, 1.0f, 1.0f);
	
	intersection = intersector.intersect(r, structure, 0xFF);

	if (intersection.type == intersection_type::none) { 
		color += contribution * sky_color;
		return false;
	} 
		
	float2 bary_2d = intersection.triangle_barycentric_coord;
	float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

	const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;

	// Calculate variables needed to solve the rendering equation
	vec_normal = (prim->normal[0] * bary_3d.x) + (prim->normal[1] * bary_3d.y) + (prim->normal[2] * bary_3d.z);
	vec_normal = normalize(intersection.object_to_world_transform * float4(vec_normal, 0.0f));
	
	// We assume for now that the surface is perfectly reflective.
	// You can chance this for materials and so forth
	// Called: BSDF: bi-directional scattering distribution function.
	r.origin = r.origin + r.direction * intersection.distance;
	seed = gid.x * (intersection.primitive_id * bary_3d.z) + gid.y * (intersection.primitive_id * bary_2d.y);
	float3 jittered_normal = normalize(vec_normal + uniform_pdf(seed) * .0);
	r.direction = reflect(r.direction, jittered_normal);
	float wi_dot_n = lambertian(r.direction, vec_normal);
			
	// Calculate all color contributions; use texture, emission if there is one
	float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
	float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
	
	color += contribution * wo_color;
	light = prim->flags[1];
	if (!light) color *= wi_dot_n;
	return true;
}

/**
float4 transport_ray_x(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	int bounces,
	thread float4& color,
	thread uint32_t& seed,
	float3 vec_light_origin
) {
	
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection;
	
	// Contribution is set to color because it will continue from previous light transport.
	float4 contribution = color;
	float4 sky_color = float4(.4f, .5f, .6f, 1.0f);
	color = float4(.0f);
	float3 normal = float3(.0f);
	float3 direction = float3(.0f);
	float3 vec_to_light = float3(.0f);
	bool visible = true;

	for (int i = 1; i <= bounces; i++) {
		// Verify if ray has intersected the geometry
		intersection = intersector.intersect(r, structure, 0xFF);
		
		direction = r.direction;
		r.direction = normalize(vec_light_origin - r.origin);
		visible = shadow_ray(r, structure, vec_light_origin);
		r.direction = direction;

		// If our ray does not hit, terminate early.
		if (intersection.type == intersection_type::none || !visible) { 
			color += contribution * visible * sky_color;
			return visible;
		}
		
		return contribution;
		
		
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
		contribution *= wo_color * wi_dot_n;
	}
	color += contribution * sky_color;
	return visible;
}
**/

void update_reservoir(
	thread float4& reservoir, 
	int light_indices, 
	float p_hat_weight, 
	thread uint32_t& seed
) {
	reservoir.x += p_hat_weight;											// w_sum - total sum of weights
	reservoir.z += 1.0f;															// m_sum - total sum of samples
	float random = rand(seed);
	if (random <= (p_hat_weight / max(reservoir.x, 1e-6f))) {
		reservoir.y = light_indices;										// sample inside of reservoir
	}
}





#endif
#endif


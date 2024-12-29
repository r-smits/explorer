#pragma once

#ifndef RayUtils_h
#define RayUtils_h

#if __METAL_VERSION__


void build_ray(thread ray& r, constant VCamera* vcamera, uint2 gid) {
	
	// Camera point needs to be within world space:
	// -1 >= x >= 1 :: Normalized
	// -1 >= y >= 1 :: We want to scale y in opposite direction for viewport coordinates
	// -1 >= z >= 0 :: The camera is pointed towards -z axis, as we use right handed
	float3 pixel = float3(float2(gid), 1.0f);
	pixel = pixel / vcamera->resolution * 2 - 1;
	pixel *= float3(1, -1, 1);

	// Projection transformations
	// orientation = matView * matProjection
	float3 vecRayDir = (vcamera->orientation * float4(pixel, 1.0f)).xyz;
	
	// Ray is modified by reference
	r.origin = vcamera->vecOrigin;
	r.direction = vecRayDir;
	r.min_distance = 0.01f;						// Set to avoid self-occlusion
	r.max_distance = FLT_MAX;
}


bool shadow_ray(
	ray r,
	instance_acceleration_structure structure,
	float3 vec_light_origin
) {
	intersector<instancing, triangle_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data> intersection;
	intersection = intersector.intersect(r, structure, 0xFF);

	if (intersection.type == intersection_type::triangle) {
		const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;
		return prim->flags[1];	
		// float3 origin = r.origin + r.direction * intersection.distance;
		// if (prim->flags[1] || abs(distance(vec_light_origin, origin)) < 0.01) return true;
	}
	return false;
}


void sample_light(
	constant Scene* scene,
	float light_index,
	float3 vec_ray_world_pos,
	thread float3& vec_world_light_pos,
	thread float3& vec_to_light,
	thread float4& vec_light_col,
	thread float& distance_to_light
) {
	// Retrieve light vertex data
	int index = int(light_index);
	vec_world_light_pos = (scene->lights[0].orientation * float4(scene->lights[0].vertices[index], 1.0f)).xyz;

	// Set variables by reference
	vec_to_light = normalize(vec_world_light_pos - vec_ray_world_pos);
	vec_light_col = scene->lights[0].attributes[index].color;
	distance_to_light = distance_squared(vec_world_light_pos, vec_ray_world_pos);
}


bool color_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	thread float4& color,
	thread float3& vec_origin,
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
	vec_origin = r.origin;		
	seed = gid.x * (intersection.primitive_id * bary_3d.z) + gid.y * (intersection.primitive_id * bary_2d.y);
	float3 jittered_normal = normalize(vec_normal + uniform_pdf(seed) * .0);
	r.direction = reflect(r.direction, jittered_normal);
	float wi_dot_n = lambertian(r.direction, vec_normal);
			
	// Calculate all color contributions; use texture, emission if there is one
	float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
	float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
	
	color += contribution * wo_color * wi_dot_n;
	light = prim->flags[1];
	return !light;
}


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





#endif
#endif


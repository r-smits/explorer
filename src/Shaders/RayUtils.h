#pragma once

#ifndef RayUtils_h
#define RayUtils_h


#if __METAL_VERSION__


ray init_ray(
	thread const float3& origin,
	thread const float3& direction,
	thread const float& min_distance,
	thread const float& max_distance
) {
	ray r;
	r.origin = origin;
	r.direction = direction;
	r.min_distance;
	r.max_distance;
	return r;
}


ray project_ray(constant VCamera* vcamera, uint2 gid) {
	float2 uv = (float2(gid) / vcamera->resolution.xy) * 2.0f - 1.0f;
    float aspect_ratio = vcamera->resolution.x / vcamera->resolution.y;
	return init_ray(
		vcamera->vecOrigin 
			+ uv.x * vcamera->fovScale * aspect_ratio * vcamera->vecRight 
			- uv.y * vcamera->fovScale * vcamera->vecUp - vcamera->vecForward * 5,
		normalize(vcamera->vecForward),
		.1f,
		FLT_MAX
	);
}


uint2 reproject_ray(thread ray& ray, constant VCamera* prev_vcamera) {
	float3 d_vec_origin = ray.origin - prev_vcamera->vecOrigin;
	float aspect_ratio = prev_vcamera->resolution.x / prev_vcamera->resolution.y;
	float2 uv = float2(
		dot(d_vec_origin, prev_vcamera->vecRight) / (prev_vcamera->fovScale * aspect_ratio),
		dot(d_vec_origin, prev_vcamera->vecUp) / prev_vcamera->fovScale
	);
	return uint2((uv * 0.5f + 0.5f) * prev_vcamera->resolution.xy);
}


LightSample sample_light(
    constant Scene* scene,
    thread const int& light_index,
    thread const float3& vec_ray_world_pos,
    thread const float3& vec_ray_normal
) {
    float3 world_pos  = (scene->lights[0].orientation * float4(scene->lights[0].vertices[light_index], 1.0f)).xyz;
    float3 direction  = normalize(world_pos - vec_ray_world_pos);
    float  dist       = distance(world_pos, vec_ray_world_pos);  // linear, not squared
    return {
        .world_pos = world_pos,
        .direction = direction,
        .color     = scene->lights[0].attributes[light_index].color,
        .distance  = dist,
        .l_dot_n   = lambertian(direction, vec_ray_normal)
    };
}


Hit intersect(
    thread ray& r,
    instance_acceleration_structure structure,
    constant Scene* scene,
    thread uint32_t& seed
) {
    intersector<instancing, triangle_data, world_space_data> intersector;
    intersector.assume_geometry_type(geometry_type::triangle);
    auto result = intersector.intersect(r, structure, 0xFF);

    if (result.type == intersection_type::none) {
        return {.did_hit = false, .color = sky_color};
    }

	const device auto* attributes = reinterpret_cast<const device PrimitiveAttributes*>(result.primitive_data);

    float3 bary_3d = float3(
		1.0 - result.triangle_barycentric_coord.x - result.triangle_barycentric_coord.y, 
		result.triangle_barycentric_coord.x, 
		result.triangle_barycentric_coord.y
	);
    
    float3 normal = normalize(result.object_to_world_transform * float4(
        attributes->normal[0].xyz * bary_3d.x + 
		attributes->normal[1].xyz * bary_3d.y + 
		attributes->normal[2].xyz * bary_3d.z, 
		.0f
	));

    float2 texture_coord = 
		attributes->txcoord[0].xy * bary_3d.x + 
		attributes->txcoord[1].xy * bary_3d.y + 
		attributes->txcoord[2].xy * bary_3d.z;
	
	float4 color = float4(
		attributes->color[0].xyz * bary_3d.x + 
		attributes->color[1].xyz * bary_3d.y + 
		attributes->color[2].xyz * bary_3d.z,
		.0f
	);

	r.origin = r.origin + r.direction * result.distance;
	return {
		.did_hit = true,
		.is_light = static_cast<bool>(attributes->flags.y),
		.normal = normal,
		.color = scene->textsample[attributes->flags.x].value.sample(sampler2d, texture_coord) + color
	};
}


Hit intersect_plane(
    thread ray& r, 
    float plane_y
) {
    // Ray-plane intersection: r.origin.y + t * r.direction.y = plane_y
    if (abs(r.direction.y) < 1e-6f) return {.did_hit = false};
    
	float distance = (plane_y - r.origin.y) / r.direction.y;
    if (distance < r.min_distance || distance > r.max_distance) return {.did_hit = false};

	// Grid pattern as the surface color
	r.origin = r.origin + r.direction * distance;
	float2 grid = abs(fract(r.origin.xz * 5.0f) - .5f);
	float line = min(grid.x, grid.y);
	return {
		.did_hit = true,
		.is_light = false,
		.normal = float3(.0f, 1.0f, .0f),
		.color = mix(float4(.3f, .3f, .3f, 1.f), float4(.1f, .1f, .1f, 1.f), step(line, 0.01f))
	};
}


Hit color_ray(
    thread ray& r,
    instance_acceleration_structure structure,
    constant Scene* scene,
    thread uint32_t& seed
) {
    Hit hit = intersect(r, structure, scene, seed);
	if (!hit.did_hit || hit.is_light) return hit;
	r.direction = reflect(r.direction, hit.normal);	
	hit.color *= lambertian(r.direction, hit.normal);
	return hit;
}


bool shadow_ray(
    thread ray& s,
    instance_acceleration_structure structure,
    float3 vec_to_light,
    float3 vec_light_origin
) {
    float3 prev_origin    = s.origin;
    float3 prev_direction = s.direction;
    float  prev_min       = s.min_distance;
    float  prev_max       = s.max_distance;

    float  light_dist = distance(vec_light_origin, s.origin);

    // Scale offset by distance to light — fixes faceting on curved geometry
    s.min_distance = max(light_dist * 1e-3f, 1e-3f);
    s.max_distance = light_dist - s.min_distance;   // don't overshoot light
    s.direction    = vec_to_light;

    intersector<instancing, triangle_data> shadow_intersector;
    shadow_intersector.assume_geometry_type(geometry_type::triangle);
    auto shadow_intersection = shadow_intersector.intersect(s, structure, 0xFF);

    bool result = (shadow_intersection.type == intersection_type::none);

    s.origin       = prev_origin;
    s.direction    = prev_direction;
    s.min_distance = prev_min;
    s.max_distance = prev_max;
    return result;
}


void update_reservoir(
	thread float4& reservoir, 
	thread const int& light_indices, 
	thread const float& p_hat_weight, 
	thread uint32_t& seed
) {
	reservoir.x += p_hat_weight;											// w_sum - total sum of weights
	reservoir.z += 1.0f;													// m_sum - total sum of samples
	if (rand(seed) <= (p_hat_weight / max(reservoir.x, 1e-6f))) {
		reservoir.y = light_indices;										// sample inside of reservoir
	}
}


float derive_roughness(thread const float4& color) {
	return max(1.0f - dot(color.rgb, luminance), 0.1f);
}


float derive_metallic(thread const float4& color) {
	float metallic = 0.0f;
    float max_c = max(color.r, max(color.g, color.b));
    float min_c = min(color.r, min(color.g, color.b));
    return (max_c - min_c) / max(max_c, 1e-4f);
}


float derive_specular(thread const float4& color) {
    return 1.0f - derive_roughness(color);
}


float disney_diffuse(float3 normal, float3 light_dir, float3 view_dir, float roughness) {
    float n_dot_l = max(dot(normal, light_dir), 0.0f);
    float n_dot_v = max(dot(normal, view_dir), 0.0f);
    
    float l_dot_h = max(dot(light_dir, normalize(light_dir + view_dir)), 0.0f);
    
    float f_d90 = 0.5f + 2.0f * roughness * l_dot_h * l_dot_h;
	float f_d = clamp(
		(1.0f + (f_d90 - 1.0f) * pow(1.0f - n_dot_l, 5.0f)) *
		(1.0f + (f_d90 - 1.0f) * pow(1.0f - n_dot_v, 5.0f)),
		0.0f, 1.0f
	);
    return f_d * n_dot_l / M_PI_F;
}


float disney_specular(
	float3 normal, 
	float3 light_dir, 
	float3 wo, 
	float roughness, 
	float metallic, 
	float specular, 
	float4 base_color
) {
    float3 h = normalize(light_dir + wo);  // half vector
    float n_dot_h = max(dot(normal, h), 0.0f);
    float n_dot_l = max(dot(normal, light_dir), 0.0f);
    float n_dot_v = max(dot(normal, wo), 0.0f);
    float l_dot_h = max(dot(light_dir, h), 0.0f);

    // GGX distribution
    float alpha = max(roughness * roughness, 0.04f);
    float denom = n_dot_h * n_dot_h * (alpha * alpha - 1.0f) + 1.0f;
    float D = alpha * alpha / (M_PI_F * denom * denom);

    // Fresnel — lerp between dielectric (0.08 * specular) and base color for metallic
    float3 f0 = mix(float3(0.08f * specular), base_color.rgb, metallic);
    float3 F = f0 + (1.0f - f0) * pow(1.0f - l_dot_h, 5.0f);

    // Geometry term (Smith GGX)
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float G = (n_dot_l / (n_dot_l * (1.0f - k) + k)) *
              (n_dot_v / (n_dot_v * (1.0f - k) + k));

    float3 specular_lobe = D * F * G / max(4.0f * n_dot_l * n_dot_v, 1e-4f);

    // Blend diffuse and specular based on metallic
    float3 diffuse = disney_diffuse(normal, light_dir, wo, roughness) * (1.0f - metallic) * base_color.rgb;
    return length(diffuse + specular_lobe) * n_dot_l;
}


float4 transport_ray(
    thread ray& r,
    instance_acceleration_structure structure,
    constant Scene* scene,
    uint2 gid,
    int bounces,
    thread uint32_t& seed
) {
    int   light_count  = scene->lights[0].vertexCount;
    float p_light_base = 1.0f / float(light_count);

    float4 throughput = float4(1.0f);
    float4 radiance   = float4(0.0f);

    Reservoir res;

    float4 first_hit_color  = float4(0.0f);
    float3 first_hit_normal = float3(0.0f);
    float3 first_hit_pos    = float3(0.0f);
    float3 first_view_dir   = float3(0.0f);
    float3 first_res_dir    = float3(0.0f);
    float3 first_res_wpos   = float3(0.0f);
    float  first_roughness  = 0.0f;
    float  first_metallic   = 0.0f;
    float  first_specular   = 0.0f;
    bool   first_hit_valid  = false;

    int i = 0;
    while (i < bounces) {
        i += 1;

        Hit hit = intersect(r, structure, scene, seed);
        // r.origin is now at the hit point

        if (!hit.did_hit) {
            radiance += throughput * sky_color;
            break;
        }
        if (hit.is_light) {
            radiance += throughput * hit.color;
            break;
        }

        float roughness = derive_roughness(hit.color);
        float metallic  = derive_metallic(hit.color);
        float specular  = derive_specular(hit.color);
        float3 view_dir = -r.direction;

        // -------------------------------------------------------
        // ReSTIR candidate — sample one light
        // -------------------------------------------------------
        int         light_index = min(int(rand(seed) * light_count), light_count - 1);
        LightSample lsample     = sample_light(scene, light_index, r.origin, hit.normal);

        float brdf_val = disney_specular(
            hit.normal, lsample.direction, view_dir,
            roughness, metallic, specular, hit.color
        );

        // lsample.l_dot_n is already clamped dot(normal, light_dir)
        float lum    = dot(lsample.color.xyz, float3(0.2126f, 0.7152f, 0.0722f));
        float p_hat  = lum * brdf_val * lsample.l_dot_n;
        float p_source = p_light_base; // uniform over lights, no area pdf stored
        float w_i    = (p_source > 0.0f) ? (p_hat / p_source) : 0.0f;

        res.update(lsample.direction, lsample.world_pos, lsample.color, w_i, seed);

        // Store the first hit's reservoir world_pos alongside direction
        // so we can fire a proper shadow ray at the end
        if (!first_hit_valid) {
            first_hit_color  = hit.color;
            first_hit_normal = hit.normal;
            first_hit_pos    = r.origin;
            first_view_dir   = view_dir;
            first_roughness  = roughness;
            first_metallic   = metallic;
            first_specular   = specular;
            first_hit_valid  = true;
            // snapshot whichever candidate is currently selected
            first_res_dir    = res.y;
            first_res_wpos   = lsample.world_pos;
        } else {
            // keep first_res_dir/wpos in sync with reservoir selection
            if (res.y.x == lsample.direction.x &&
                res.y.y == lsample.direction.y &&
                res.y.z == lsample.direction.z) {
                first_res_wpos = lsample.world_pos;
            }
        }

        // -------------------------------------------------------
        // NEE + MIS at this bounce
        // -------------------------------------------------------
        bool visible = shadow_ray(r, structure, lsample.direction, lsample.world_pos);

        if (visible && lsample.l_dot_n > 0.0f) {
            float mis_weight = p_source / (p_source + brdf_val * lsample.l_dot_n + 1e-6f);
            radiance += throughput
                      * hit.color
                      * brdf_val
                      * lsample.color
                      * lsample.l_dot_n
                      * mis_weight
                      / (p_source + 1e-6f);
        }

        // -------------------------------------------------------
        // Sample next bounce direction
        // -------------------------------------------------------
        float3 jittered_normal = normalize(hit.normal + uniform_pdf(seed) * 0.1f);
        float3 new_dir         = reflect(r.direction, jittered_normal);
        float  cos_theta       = max(dot(hit.normal, new_dir), 0.0f);
        float  pdf_brdf        = cos_theta / M_PI_F;

        if (pdf_brdf <= 0.0f) break;

        throughput *= hit.color * (brdf_val / pdf_brdf) * cos_theta;

        r.origin    = r.origin + hit.normal * 1e-4f;
        r.direction = new_dir;
    }

    // -------------------------------------------------------
    // ReSTIR: evaluate reservoir's selected direction
    // against the first indirect hit
    // -------------------------------------------------------
    if (first_hit_valid && res.m > 0) {
		float brdf_val = disney_specular(
			first_hit_normal, res.y, first_view_dir,
			first_roughness, first_metallic, first_specular, first_hit_color
		);
		float n_dot_l = max(dot(first_hit_normal, res.y), 0.0f);
	
		float3 saved_origin = r.origin;
		r.origin = first_hit_pos + first_hit_normal * max(distance(res.p, first_hit_pos) * 1e-3f, 1e-3f);
	
		bool visible = shadow_ray(r, structure, res.y, res.p);  // use res.p directly
		r.origin = saved_origin;
	
		if (visible && n_dot_l > 0.0f) {
			float lum   = dot(res.c.xyz, luminance);             // use res.c for Le
			float p_hat = lum * brdf_val * n_dot_l;
			float W     = (p_hat > 0.0f) ? (res.w_sum / (res.m * p_hat)) : 0.0f;
	
			radiance += first_hit_color * brdf_val * res.c * W * n_dot_l;
		}
	}
}


// float4 transport_ray(
// 	thread ray& r,
// 	instance_acceleration_structure structure,
// 	constant Scene* scene,
// 	uint2 gid,
// 	int bounces,
// 	thread uint32_t& seed
// ) {	
// 	thread int light_count = scene->lights[0].vertexCount;
// 	thread float p_light    = 1.0f / light_count;
// 	thread float4 contribution = float(1.0f);
// 	thread float4 weighted = float(.0f);

// 	thread int i = 0;
// 	while (i < bounces) {
// 		i += 1;

// 		Hit hit = intersect(r, structure, scene, seed);
// 		if (!hit.did_hit || hit.is_light) break;

// 		float roughness = derive_roughness(hit.color);
// 		float metallic  = derive_metallic(hit.color);
// 		float specular  = derive_specular(hit.color);
		
// 		int light_count  = scene->lights[0].vertexCount;
// 		int light_index  = min(int(rand(seed) * light_count), light_count - 1);
// 		LightSample lsample = sample_light(scene, light_index, r.origin, hit.normal);

// 		float p_light    = 1.0f / light_count;
// 		float p_brdf     = disney_specular(
// 			hit.normal, 
// 			lsample.direction, 
// 			-r.direction, 
// 			roughness,
// 			metallic,
// 			specular,
// 			hit.color
// 		);
// 		p_brdf = max(dot(hit.normal, lsample.direction), 0.0f) / M_PI_F;
// 		float mis_weight = p_light / (p_light + p_brdf);

// 		float3 jittered_normal = normalize(hit.normal + uniform_pdf(seed) * 0.1f);
// 		r.direction = reflect(r.direction, jittered_normal);

// 		bool visible = shadow_ray(r, structure, lsample.direction, lsample.world_pos);
// 		float4 weighted = weighted + (float(visible) * lsample.l_dot_n * hit.color * lsample.color * mis_weight);

// 		update_reservoir(contribution, light_index, dot(weighted.xyz, luminance), seed);
// 	}

// 	return weighted * contribution.w * sky_color;
// }


#endif
#endif


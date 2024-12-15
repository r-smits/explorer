#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"

// Object constants

void transport(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid,
	thread int bounces,
	thread float4& color,
	thread uint32_t& seed,
	thread bool unshadowed_light_contribution,
	thread float3& normal,
	thread bool& terminate_flag,
	thread bool inverse_square
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
			terminate_flag = (i == 1);
			return; 
		} 
		if (intersection.type == intersection_type::triangle) {

			float2 bary_2d = intersection.triangle_barycentric_coord;
			float3 bary_3d = float3(1.0 - bary_2d.x - bary_2d.y, bary_2d.x, bary_2d.y);

			const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;

			// Calculate variables needed to solve the rendering equation
			normal = (prim->normal[0] * bary_3d.x) + (prim->normal[1] * bary_3d.y) + (prim->normal[2] * bary_3d.z);
			normal = normalize((intersection.object_to_world_transform * float4(normal, 0.0f)).xyz);
			seed = gid.x * (intersection.primitive_id * bary_3d.z * i) + gid.y * (intersection.primitive_id * bary_2d.y);
			
			// We assume for now that the surface is perfectly reflective.
			// You can chance this for materials and so forth
			// Called: BSDF: bi-directional scattering distribution function.
			r.origin = r.origin + r.direction * intersection.distance; 
			float3 jittered_normal = normalize(normal + uniform_pdf(seed) * .2);
			r.direction = reflect(r.direction, jittered_normal);

			float3 wi = normalize(r.direction);
			float wi_dot_n = (inverse_square) ? cos_inverse_square(intersection.distance, wi, normal) : cos(wi, normal);
			
			// Calculate all color contributions; use texture, emission if there is one
			float2 txcoord = (prim->txcoord[0] * bary_3d.x) + (prim->txcoord[1] * bary_3d.y) + (prim->txcoord[2] * bary_3d.z);
			float4 wo_color = scene->textsample[prim->flags[0]].value.sample(sampler2d, txcoord) + prim->color[0];
			
			contribution *= wo_color * wi_dot_n;
			// color *= (unshadowed_light_contribution) ? 1 : visibility_check();
			color += prim->flags[1] * wo_color;
		}
	}
	color += contribution * sky_color;
}

[[kernel]]
void computeKernel(
	texture2d<float, access::write> buffer								[[ texture(0) ]],
	instance_acceleration_structure structure							[[ buffer(1)	]],
	constant Scene* scene																	[[ buffer(2)	]],
	uint2 gid																							[[ thread_position_in_grid	]] 
) {
	
	// Check if instance acceleration structure was built succesfully
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
		if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, gid);
		return;
	}
	
	// Initialize default parameters
	thread uint32_t seed = 0;
	thread int bounces = 3;
	thread float3 normal = float3(0.0f);

	// Initialize ReSTIR variables (later to reconsider the memory scope)
	// N good samples defined in constant address space, maybe there is a better way of doing things
	thread int m = 8;										// Number of bad samples
	//thread int n = 1;									// Number of good samples
	
	// In the future we need to pass it in as an argument to the kernel shader
	// Otherwise you have to instantiate them here
	Reservoir r1;

	// Build ray. Ray shoots out from point (gid). Camera is a grid, point is a coordinate on the grid. 
	ray r;
	ray x;
	build_ray(r, scene->vcamera, gid);
		
	// Shoot initial ray from the camera into the scene. 
	// This will set the ray, color and seed by reference.
	bool terminate_flag = false;
	transport(r, structure, scene, gid, bounces, color, seed, true, normal, terminate_flag, false);
	if (terminate_flag) {
		buffer.write(color, gid);
		return;
	}
	x = r;
	
	// ReSTIR GI implementation
	for (int i = 0; i < m; i += 1) {

		// 1. Calculate the samples from the uniform pdf.
		// All samples in a uniform have equal weights, so we don't have to calculate them. The sum of weights would be 1.
		//float3 pdf_sample = normalize(normal + uniform_pdf(seed) * 1); 
		float3 pdf_sample = uniform_pdf(seed);	
		
		// 2. Calculate the weights of the complex pdf through the unshadowed light contribution.
		// For the complex pdf, we take the color contribution as the weights, required for the re-sample. 
		float4 sample_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
		x.direction = pdf_sample;
		x.origin = r.origin;
		transport(x, structure, scene, gid, 2, sample_color, seed, true, normal, terminate_flag, true);
		float pdf_weight = intensity(sample_color);

		// 3. Build the complex pdf to re-sample the samples from.
		// Sample color intensity are weights for the complex pdf.
		r1.update(pdf_sample, pdf_weight, seed);
	}

	// 4. Take the inverse of the sample as a weight and multiply it by the average weight (color) of the bad samples
	// To compensate for the weight being more likely to be picked.
	float4 total_sample_color = float4(0.0f);
	float average_weight_samples = r1.w_sum / r1.m;
	
	// r1
	x.direction = r1.y;
	float4 sample_color = float4(0.0f);
	transport(x, structure, scene, gid, bounces, sample_color, seed, true, normal, terminate_flag, true);
	x = r;
	
	float r1_norm_weight = average_weight_samples / r1.w; // intensity(sample_color); <- Should be this but minimal difference

	total_sample_color += sample_color * r1_norm_weight;

	color *= total_sample_color;
	buffer.write(color, gid);
}

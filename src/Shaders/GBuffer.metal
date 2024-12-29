#include <metal_stdlib>
#include <metal_raytracing>
using namespace metal;
using namespace raytracing;

#import "../src/Shaders/ShaderTypes.h"
#import "../src/Shaders/RTUtils.h"


// Texture coordinates
// (0) World position
// (1) World normal
// (2) GID Color
void shoot_ray(
	thread ray& r,
	thread instance_acceleration_structure& structure,
	constant Scene* scene,
	uint2 gid
) {
	
	intersector<instancing, triangle_data, world_space_data> intersector;	
	intersector.assume_geometry_type(geometry_type::triangle);
	intersection_result<instancing, triangle_data, world_space_data> intersection = intersector.intersect(r, structure, 0xFF);

	float4 contribution = float4(1.0f);
	float4 saturated = float4(1.0f);
	float4 sky_color = float4(.4f, .5f, .6f, 1.0f);

	if (intersection.type == intersection_type::none) {
		contribution *= sky_color;
		scene->textreadwrite[GBufferIds::col].value.write(contribution, gid);
		scene->textreadwrite[GBufferIds::norm].value.write(saturated, gid);
		scene->textreadwrite[GBufferIds::pos].value.write(saturated, gid);

		return; 
	} 
	
	if (intersection.type == intersection_type::triangle) {
		
		float4 hit = float4(r.origin + r.direction * intersection.distance, 0.0f);

		const device PrimitiveAttributes* prim = (const device PrimitiveAttributes*) intersection.primitive_data;
		
		float2 bary2 = intersection.triangle_barycentric_coord;
		float3 bary3 = float3(1.0 - bary2.x - bary2.y, bary2.x, bary2.y);
		
		float3 normal3 = (prim->normal[0] * bary3.x) + (prim->normal[1] * bary3.y) + (prim->normal[2] * bary3.z);
		normal3 = normalize(intersection.object_to_world_transform * float4(normal3, 0.0f));

		float2 txcoord = (prim->txcoord[0] * bary3.x) + (prim->txcoord[1] * bary3.y) + (prim->txcoord[2] * bary3.z);
		float4 wo_color = scene->textsample[prim->flags[PrimFlagIds::textid]].value.sample(sampler2d, txcoord) + prim->color[0];
		
		float3 wi_dir = normalize(reflect(r.direction, normal3));
		float cosine = lambertian(wi_dir, normal3);

		// float3 wo_dir = 1 - normalize(r.direction);
		contribution *= wo_color * cosine;
		// contribution = prim->flags[PrimFlagIds::emissive] * wo_color + contribution * wo_color;
		scene->textreadwrite[GBufferIds::pos].value.write(hit, gid);
		scene->textreadwrite[GBufferIds::norm].value.write(float4(normal3, .0f), gid);
		scene->textreadwrite[GBufferIds::col].value.write(contribution, gid);
	}
}


[[kernel]]
void g_buffer(
	texture2d<float, access::write> buffer								[[ texture(0) ]],
	instance_acceleration_structure structure							[[ buffer(1)	]],
	constant Scene* scene																	[[ buffer(2)	]],
	uint2 gid																							[[ thread_position_in_grid	]] 
) {
	
	float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);
		if (is_null_instance_acceleration_structure(structure)) {
		buffer.write(color, gid);
		return;
	}

	ray r;	
	build_ray(r, scene->vcamera, gid);
	shoot_ray(r, structure, scene, gid);
	

	float4 value = scene->textreadwrite[GBufferIds::col].value.read(gid);
	buffer.write(value, gid);
}


/**
BRDF stuff that I need to understand at some point
**/

/**
float3 fresnelSchlick(float3 f0, float3 f90, float u)
{
    return f0 + (f90 - f0) * pow(1 - u, 5);
}

// Disney's diffuse term. Based on https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
float disneyDiffuseFresnel(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float fd90 = 0.5 + 2 * LdotH * LdotH * linearRoughness;
    float fd0 = 1;
    float lightScatter = fresnelSchlick(fd0, fd90, NdotL).r;
    float viewScatter = fresnelSchlick(fd0, fd90, NdotV).r;
    return lightScatter * viewScatter;
}

float3 evalDiffuseDisneyBrdf(ShadingData sd, LightSample ls)
{
    return disneyDiffuseFresnel(sd.NdotV, ls.NdotL, ls.LdotH, sd.linearRoughness) * M_INV_PI * sd.diffuse.rgb;
}

// Lambertian diffuse
float3 evalDiffuseLambertBrdf(ShadingData sd, LightSample ls)
{
    return sd.diffuse.rgb * (1 / M_PI);
}

// Frostbites's diffuse term. Based on https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
float3 evalDiffuseFrostbiteBrdf(ShadingData sd, LightSample ls)
{
    float energyBias = lerp(0, 0.5, sd.linearRoughness);
    float energyFactor = lerp(1, 1.0 / 1.51, sd.linearRoughness);

    float fd90 = energyBias + 2 * ls.LdotH * ls.LdotH * sd.linearRoughness;
    float fd0 = 1;
    float lightScatter = fresnelSchlick(fd0, fd90, ls.NdotL).r;
    float viewScatter = fresnelSchlick(fd0, fd90, sd.NdotV).r;
    return (viewScatter * lightScatter * energyFactor * M_INV_PI) * sd.diffuse.rgb;
}

float3 evalDiffuseBrdf(ShadingData sd, LightSample ls)
{
#if DiffuseBrdf == DiffuseBrdfLambert
    return evalDiffuseLambertBrdf(sd, ls);
#elif DiffuseBrdf == DiffuseBrdfDisney
    return evalDiffuseDisneyBrdf(sd, ls);
#elif DiffuseBrdf == DiffuseBrdfFrostbite
    return evalDiffuseFrostbiteBrdf(sd, ls);
#endif
}

float evalGGX(float roughness, float NdotH)
{
    float a2 = roughness * roughness;
    float d = ((NdotH * a2 - NdotH) * NdotH + 1);
    return a2 / (d * d);
}

float evalSmithGGX(float NdotL, float NdotV, float roughness)
{
    // Optimized version of Smith, already taking into account the division by (4 * NdotV)
    float a2 = roughness * roughness;
    // `NdotV *` and `NdotL *` are inversed. It's not a mistake.
    float ggxv = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    float ggxl = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    return 0.5f / (ggxv + ggxl);

}

float3 evalSpecularBrdf(ShadingData sd, LightSample ls)
{
    float roughness = sd.roughness;
    
    float D = evalGGX(roughness, ls.NdotH);
    float G = evalSmithGGX(ls.NdotL, sd.NdotV, roughness);
    float3 F = fresnelSchlick(sd.specular, 1, max(0, ls.LdotH));
    return D * G * F * M_INV_PI;
}

**/

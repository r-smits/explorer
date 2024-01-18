#include <metal_stdlib>

using namespace metal;

constant float PI = 3.1415926535897932384626433832795;

struct Light {
  float3 position;	// {x, y, z}
	float3 color;			// {r, g, b}
	float4 factors;		// {brightness, ambientIntensity, diffuseIntensity, specularIntensity}
};

struct Projection {
	simd::float4x4 camera; 
	simd::float4x4 model;
	simd::float3 cameraPosition;
};

struct VertexInput {
	float4 position		[[attribute(0)]];		// {x, y, z, w}
  float3 color			[[attribute(1)]];		// {r, g, b}
	float2 texture		[[attribute(2)]];		// {x, y}
	float3 normal			[[attribute(3)]];		// v{x, y, z}
};

struct VertexOutput {
  float4 position		[[position]];				// {x, y, z, w}
  float2 texture;
	float3 color;
	float3 worldPosition;
	float3 surfaceNormal;
	float3 toCameraVec;
};

struct Material {
	simd::float4 color;
  simd::float3 ambient;
	simd::float3 diffuse;
	simd::float4 specular;		// {x, y, z, shininess}
	bool useColor;
	bool useLight;
};

constexpr sampler sampler2d(address::clamp_to_edge, filter::linear); 

VertexOutput vertex vertexMainGeneral(
    VertexInput input [[stage_in]],
		constant Projection& projection [[buffer(1)]]
) {
	VertexOutput output;
  float4 worldPosition = projection.model * input.position;
	output.position = projection.camera * worldPosition;
	output.worldPosition = worldPosition.xyz;
		
	output.color = input.color;
	output.texture = input.texture;

	output.surfaceNormal = (projection.camera * projection.model * float4(input.normal, 1.0)).xyz;
	output.toCameraVec = projection.cameraPosition - output.worldPosition;
	return output;
}

half4 fragment fragmentMainGeneral(
		VertexOutput frag [[stage_in]],
		constant Light& light [[buffer(1)]],
		constant Material& material [[buffer(2)]],
		texture2d<float> texture [[texture(0)]]
) {
		// Color or Texture
		float4 color = material.useColor ? material.color : texture.sample(sampler2d, frag.texture);
		if (!material.useLight) return half4(color);

		// float lBrightness = 100 / length(frag.position.xyz - light.position.xyz);

		float brightness = light.factors.x;
		float fAmbient = light.factors.y;
		float fDiffuse = light.factors.z;
		float fSpecular = light.factors.w;

		float3 mSpecular = material.specular.xyz;
		float shininess = material.specular.w;

		float3 pixelBright = light.color * brightness;
			
		// ambient
		float3 ambient = clamp(material.ambient * fAmbient * pixelBright, 0.0, 1.0);
		
		// diffuse
		float3 unitToLightVec = normalize(light.position - frag.worldPosition);
		float3 unitSurfaceNormal = normalize(frag.surfaceNormal);
		float diffuseProduct = max(dot(unitSurfaceNormal, unitToLightVec), 0.0);
		float3 diffuse = clamp(material.diffuse * diffuseProduct * fDiffuse * pixelBright, 0.0, 1.0);
	
		// specular
		float3 unitToCameraVec = normalize(frag.toCameraVec);
		float3 unitReflectVec = normalize(reflect(-unitToLightVec, unitSurfaceNormal));
		float specularRefract = pow(max(dot(unitReflectVec, unitToCameraVec), 0.0), shininess);
		float3 specular = clamp(mSpecular * fSpecular * specularRefract * pixelBright, 0.0, 1.0);
		
		float3 phong = ambient + diffuse + specular;	
		color.rgb *= phong;
		return half4(color);
}


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
	// float3 toCameraVec;
};

struct Material {
	simd::float4 color;
  simd::float3 ambient;
	simd::float3 diffuse;
	simd::float3 specular;
	bool useColor; 
  //float shininess;
	//simd::float2 texture;
};

VertexOutput vertex vertexMainGeneral(
    VertexInput input [[stage_in]],
		constant Projection& projection [[buffer(1)]]
) {
	VertexOutput output;

	//half4x4 model = half4x4(projection.model);
	//half4x4 camera = half4x4(projection.camera);
	//half4 position = half4(input.position);
	//half4 normal = half4(half3(input.normal), 1.0);

  float4 worldPosition = projection.model * input.position;
	output.position = projection.camera * worldPosition;
	output.worldPosition = worldPosition.xyz;
		
	output.color = input.color;
	output.texture = input.texture;

	output.surfaceNormal = (projection.model * float4(input.normal, 1.0)).xyz; // you can do this outside.
	return output;
}

half4 fragment fragmentMainGeneral(
		VertexOutput frag [[stage_in]],
		constant Light& light [[buffer(1)]],
		constant Material& material [[buffer(2)]],
		sampler sampler2D [[sampler(0)]],
		texture2d<float> texture [[texture(0)]]
) {
		// Color or Texture
		float4 color = material.useColor ? material.color : texture.sample(sampler2D, frag.texture);

		// float lBrightness = 100 / length(frag.position.xyz - light.position.xyz);

		float brightness = light.factors.x;
		float fAmbient = light.factors.y;
		float fDiffuse = light.factors.z;
		float fSpecular = light.factors.w;

		float3 pixelBrightness = light.color * brightness;
			
		// ambient
		float3 ambient = clamp(material.ambient * fAmbient * pixelBrightness, 0.0, 1.0);
		
		// diffuse
		float3 unitToLightVec = normalize(light.position - frag.worldPosition);
		float3 unitSurfaceNormal = normalize(frag.surfaceNormal);
		float diffuseProduct = max(dot(unitSurfaceNormal, unitToLightVec), 0.0);
		float3 diffuse = clamp(material.diffuse * diffuseProduct * fDiffuse * pixelBrightness, 0.0, 1.0);
						
		float3 phong = ambient + diffuse; // diffuse; //ambient; //+ diffuse; // specular;	
		color.rgb *= phong;
		//color.rgb *= lBrightness;

		return half4(color);
}


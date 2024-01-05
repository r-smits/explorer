#include <metal_stdlib>

using namespace metal;

constant float PI = 3.1415926535897932384626433832795;

struct Light {
		float4 position;
};

struct VertexInput {
		float4 position [[attribute(0)]];
    float3 color [[attribute(1)]];
};

struct VertexOutput {
    float4 position [[position]];
    half3 color;
};

VertexOutput vertex vertexMainGeneral(
    VertexInput input [[stage_in]],
    constant float4x4& transform [[buffer(1)]]
) {
    VertexOutput vertexOutput;
    vertexOutput.position = float4(half4x4(transform) * half4(input.position));
    vertexOutput.color = half3(input.color);
    return vertexOutput;
}

half4 fragment fragmentMainGeneral(
		VertexOutput frag [[stage_in]], 
		constant Light& light [[buffer(1)]]
) {
		float brightness = 100 / length(frag.position.xyz - light.position.xyz);
    return half4(frag.color * brightness, 1.0);
}


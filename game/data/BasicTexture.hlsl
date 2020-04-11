 struct VOut {
     float4 position : SV_POSITION;
     float4 color : COLOR0;
     float2 uv : TEXCOORD0;
 };

cbuffer Scene : register(b0) {
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

cbuffer Object : register(b1) { float4x4 ModelMatrix; };

VOut VSMain(float3 position : POSITION, float4 color : COLOR0, float2 uv : TEXCOORD0) {
    VOut output;

    output.position = mul(float4(position, 1.0), ModelMatrix);
    // output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.color = color;
    output.uv = uv;

    return output;
}

Texture2D Texture0 : register(t0);
sampler Sampler0 : register(s0);

float4 PSMain(VOut input) : SV_TARGET { return input.color * Texture0.Sample(Sampler0, input.uv); }

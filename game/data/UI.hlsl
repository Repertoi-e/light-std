cbuffer Scene : register(b0) { float4x4 MVP; };

struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VOut VSMain(float2 position : POSITION, float4 color : COLOR0, float2 uv : TEXCOORD0) {
    VOut output;

    output.position = mul(MVP, float4(position.xy, 0.f, 1.f));
    output.color = color;
    output.uv = uv;

    return output;
}

sampler Sampler0;
Texture2D Texture0;

float4 PSMain(VOut input) : SV_Target { return input.color * Texture0.Sample(Sampler0, input.uv); }

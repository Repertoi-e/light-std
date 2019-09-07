struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

VOut VSMain(float3 position : POSITION, float4 color : COLOR0, float2 uv : TEXCOORD0) {
    VOut output;

    output.position = float4(position, 1.0);
    output.color = color;
    output.uv = uv;

    return output;
}

Texture2D Texture0 : register(t0);
sampler Sampler0 : register(s0);

float4 PSMain(VOut input) : SV_TARGET { return input.color * Texture0.Sample(Sampler0, input.uv); }

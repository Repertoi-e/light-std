cbuffer Scene : register(b0) {
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

cbuffer Object : register(b1) { float4x4 ModelMatrix; }

struct VIn {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VSMain(VIn input) {
    VOut output;

    output.position = mul(float4(input.position, 1.0), ModelMatrix);
    output.position = mul(output.position, ViewMatrix);
    output.position = mul(output.position, ProjectionMatrix);
    output.color = input.color;

    return output;
}

float4 PSMain(VOut input) : SV_TARGET { return input.color; }

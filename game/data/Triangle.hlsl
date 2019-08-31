struct VOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VSMain(float3 position : POSITION, float4 color : COLOR) {
    VOut output;

    output.position = float4(position, 1.0);
    output.color = color;

    return output;
}

float4 PSMain(VOut input) : SV_TARGET { return input.color; }

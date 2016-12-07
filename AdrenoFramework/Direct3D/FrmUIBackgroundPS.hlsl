cbuffer UIBackgroundConstantBuffer
{
    float4 vBackground;
};

struct PixelShaderInput
{
    float4 vVertexPos : SV_POSITION;    
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return vBackground;
}
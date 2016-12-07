Texture2D overlayTexture : register(t0);
SamplerState overlaySampler: register(s0);

struct PixelShaderInput
{
    float4 vVertexPos : SV_POSITION;
    float2 vTexCoord : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return overlayTexture.Sample(overlaySampler, input.vTexCoord);
}
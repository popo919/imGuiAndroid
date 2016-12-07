Texture2D fontTexture : register(t0);
SamplerState fontSampler: register(s0);

struct PixelShaderInput
{
    float4 vVertexPos : SV_POSITION;    
    float4 vVertexColor: COLOR0;
    float2 vVertexTex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return (input.vVertexColor * fontTexture.Sample(fontSampler, input.vVertexTex));
}
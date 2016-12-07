cbuffer FontConstantBuffer
{
    float4 vScale;
    float4 vOffset;
};

struct VertexShaderInput
{
    float2 vVertexPos : POSITION;
    float4 vVertexColor: COLOR0;
    float2 vVertexTex : TEXCOORD0;
};

struct PixelShaderInput
{
    float4 vVertexPos : SV_POSITION;
    float4 vVertexColor: COLOR0;
    float2 vVertexTex : TEXCOORD0;
};

PixelShaderInput main( VertexShaderInput input )
{
    PixelShaderInput vso;
    // Transform the position
    float2 vVertex   = input.vVertexPos * vScale.xy + vOffset.xy;
    vso.vVertexPos = float4( vVertex, 0.5, 1.0 );

    // Pass through color and texture coordinates
    vso.vVertexColor = input.vVertexColor;
    vso.vVertexTex = (input.vVertexTex * vScale.zw) + vOffset.zw;

    return vso;
}

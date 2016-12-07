struct VertexShaderInput
{
    float2 vVertexPos : POSITION;
};

struct PixelShaderInput
{
    float4 vVertexPos : SV_POSITION;
};

PixelShaderInput main( VertexShaderInput input )
{
    PixelShaderInput vso;
    vso.vVertexPos = float4( input.vVertexPos, 0.5, 1.0 );
    
    return vso;
}

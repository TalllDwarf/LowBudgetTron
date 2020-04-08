Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{

#ifdef ALPHA_TEST

    clip(t1.Sample(s1, input.texCoord).a -0.01);

#endif

    return float4(input.color.rgb, input.color.a * t1.Sample(s1, input.texCoord).a);
}
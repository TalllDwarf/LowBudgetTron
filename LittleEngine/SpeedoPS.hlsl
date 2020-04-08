Texture2D shaderTextures[2] : register(t0);
SamplerState s1 : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 viewDirection : TEXCOORD1;
    float2 addToTex : TEXCOORD2;
};

cbuffer LightBuffer : register(b1)
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 lightColor[2];
    float3 lightDirection;
    float specularPower;
    float3 lightPosition;
    float range;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 color;
    float4 highlighting;

    color = shaderTextures[0].Sample(s1, input.texCoord) * 0.8f;
    color = color + (lightColor[0] * 0.2f);

    /*
    if(color.r < 0.90f)
    {
        highlighting = shaderTextures[1].Sample(s1, (input.texCoord));
        highlighting = lightColor[0] * highlighting.r;
        color = color + highlighting;
    } 
    */


    return color;
}
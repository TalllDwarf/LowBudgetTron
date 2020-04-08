Texture2D shaderTextures[2] : register(t0);
SamplerState s1 : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
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
    float4 textureColor;
    float4 color;

    float2 uv = input.texCoord;
    uv.x += input.addToTex.x;

    textureColor = shaderTextures[0].Sample(s1, uv) * 0.5f;


    uv = input.texCoord;
    uv.y += input.addToTex.y;

    float4 color1 = shaderTextures[0].Sample(s1, uv);

    textureColor += (color1 * 0.5f);

    //Get the current light color then multiply it with the Green value in our texture
    if (color1.a > 0.1f)
    {
        color = lightColor[0] * color1.y;
        color = color * textureColor;
    }

    return color;
}
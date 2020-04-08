Texture2D shaderTextures[3] : register(t0);
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
    float4 textureColor;
    float4 maskColour;
    float4 color;

    textureColor = (shaderTextures[0].Sample(s1, input.texCoord) * 0.5);
    maskColour = (shaderTextures[2].Sample(s1, (input.texCoord + input.addToTex)) * 0.5);

    textureColor += maskColour;

    float4 color1 = shaderTextures[1].Sample(s1, input.texCoord);
    
    color = textureColor * 0.5f;

    color = color * (lightColor[0] * color1.y);

    return color;
}
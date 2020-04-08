Texture2D shaderTextures[3] : register(t0);
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

    textureColor = shaderTextures[0].Sample(s1, input.texCoord);

#ifdef ALPHA_TEST

    clip(textureColor.a -0.01);

#endif

    textureColor * 0.7;
    textureColor += (shaderTextures[1].Sample(s1, (input.texCoord + input.addToTex)) * 0.3);

    float4 color1 = shaderTextures[2].Sample(s1, input.texCoord);

    //Get the current light color then multiply it with the Green value in our texture
    color = lightColor[0] * color1.y;
    
    color = color * textureColor;

    return color;
}
Texture2D shaderTextures[3] : register(t0);
SamplerState s1 : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDirection : TEXCOORD1;
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
    float4 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float lightIntensity;
    float4 color;
    float4 specularIntensity;
    float3 reflection;
    float4 specular;

    // Sample the texture pixel at this location.
    textureColor = shaderTextures[0].Sample(s1, input.texCoord);

    // Invert the light direction for calculations.
    lightDir = -lightDirection;

    // Calculate the amount of light on this pixel based on the bump map normal value.
    lightIntensity = saturate(dot(input.normal, lightDir));

    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    if(lightIntensity > 0.0f)
    {

        // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
        color = (lightColor[0] * lightIntensity);

        color += (lightColor[0] * 0.4f);

        // Saturate the ambient and diffuse color.
        color = saturate(color);

        // Calculate the reflection vector based on the light intensity, normal vector, and light direction.
        reflection = normalize(2 * lightIntensity * input.normal - lightDir);

        // Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
        specular = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
	
    }

    // Combine the final bump light color with the texture color.
    color = color * textureColor;

    // Add the specular component last to the output color.
    color = saturate(color + specular);
	
    return color;
}
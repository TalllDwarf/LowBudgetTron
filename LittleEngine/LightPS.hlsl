Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 normal : NORMAL;
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
    float4 diffuse = t1.Sample(s1, input.TexCoord);

    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    
    //Create the vector between light position and pixels position
    float3 lightToPixelVec = lightPosition.xyz - input.worldPos.xyz;
        
    //Find the distance between the light pos and pixel pos
    float d = length(lightToPixelVec);
    
    //Create the ambient light
    float3 finalAmbient = diffuse * ambientColor;

    //If pixel is too far, return pixel color with ambient light
    if (d > range)
        return float4(finalAmbient, diffuse.a);
        
    //Turn lightToPixelVec into a unit length vector describing
    //the pixels direction from the lights position
    lightToPixelVec /= d;
    
    //Calculate how much light the pixel gets by the angle
    //in which the light strikes the pixels surface
    float howMuchLight = dot(lightToPixelVec, input.normal);

    //If light is striking the front side of the pixel
    if (howMuchLight > 0.0f)
    {
        //Add light to the finalColor of the pixel
        finalColor += howMuchLight * diffuse * diffuse * diffuseColor;
        
        //Calculate Light's Falloff factor
        //finalColor /= att[0] + (att[1] * d) + (att[2] * (d * d));
    }
        
    //make sure the values are between 1 and 0, and add the ambient
    finalColor = saturate(finalColor + finalAmbient);
    
    //Return Final Color
    return float4(finalColor, diffuse.a);
}
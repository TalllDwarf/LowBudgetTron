struct VS_INPUT
{
    float4 pos : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 viewDirection : TEXCOORD1;
    float2 addToTex : TEXCOORD2;
};

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewMat;
    float4x4 projMat;
    float4 cameraPosition;
    float2 uvMovement;
}

cbuffer ConstantBuffer : register(b2)
{
    float4x4 worldMatrix;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    //Change the position vector to be 4 units
    input.pos.w = 1.0f;

    //Calculate the vertex world position
    output.pos = mul(input.pos, worldMatrix);
    output.pos = mul(output.pos, viewMat);
    output.pos = mul(output.pos, projMat);

    output.addToTex = uvMovement;

    output.texCoord = input.texCoord;

    float4 worldPosition = mul(input.pos, worldMatrix);

	//determine the viewing direction based on the position of the camera and the position og the vertex in the world
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;

    //Normalize the viewing direction vector
    output.viewDirection = normalize(output.viewDirection);

    return output;
}
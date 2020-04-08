struct VS_INPUT
{
    float4 pos : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
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

    output.pos = mul(input.pos, worldMatrix);
    output.pos = mul(output.pos, viewMat);
    output.pos = mul(output.pos, projMat);

    output.texCoord = input.texCoord;

    //Calculate the normal vector
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    output.worldPos = mul(input.pos, worldMatrix);

    return output;
}
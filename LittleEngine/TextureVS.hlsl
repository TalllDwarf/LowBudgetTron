struct VS_INPUT
{
    float4 pos : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD;
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
    return output;
}
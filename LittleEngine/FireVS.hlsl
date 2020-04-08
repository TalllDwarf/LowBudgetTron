struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float2 texCoords1 : TEXCOORD1;
	float2 texCoords2 : TEXCOORD2;
	float2 texCoords3 : TEXCOORD3;
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
	float frameTime;
	float3 scrollSpeed;
	float3 scales;
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

	output.texCoord = input.texCoord;

	//compute texture coords
	output.texCoords1 = (input.texCoord * scales.x);
	output.texCoords1.y = output.texCoords1.y + (frameTime * scrollSpeed.x);

	output.texCoords2 = (input.texCoord * scales.y);
	output.texCoords2.y = output.texCoords2.y + (frameTime * scrollSpeed.y);

	output.texCoords3 = (input.texCoord * scales.z);
	output.texCoords3.y = output.texCoords3.y + (frameTime * scrollSpeed.z);


	return output;
}
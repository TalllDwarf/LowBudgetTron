#pragma once
#include "stdafx.h"

/*
	Rotation vector, to include rotation steps
	If needed
*/

class LETransform
{
public:
	LETransform(XMFLOAT4 position, XMFLOAT3 rotation, XMFLOAT3 scale);
	~LETransform();

	XMFLOAT4 Position;
	XMFLOAT3 Rotation;
	XMFLOAT3 Scale;

	XMFLOAT3 FirstRotation;

	void MoveActorLocation(XMFLOAT4 addMovement);

	void MoveActorLocation(float x, float y, float z);

	//Creates and returns the world matrix for the Transform
	XMMATRIX GetWorldMatrix();


	static XMMATRIX GetOffsetMatrix(LETransform part, LETransform player)
	{
		XMMATRIX outMatrix = XMMatrixScaling(part.Scale.x, part.Scale.y, part.Scale.z);
		outMatrix = (outMatrix * XMMatrixRotationRollPitchYaw(part.FirstRotation.x, part.FirstRotation.y, part.FirstRotation.z));
		outMatrix = (outMatrix * XMMatrixRotationRollPitchYaw(part.Rotation.z, part.Rotation.y, part.Rotation.x));		
		outMatrix = (outMatrix * XMMatrixTranslation(part.Position.x, part.Position.y, part.Position.z - 2.0f));

		outMatrix = (outMatrix * XMMatrixRotationRollPitchYaw(player.Rotation.z, player.Rotation.y, player.Rotation.x));
		outMatrix = (outMatrix * XMMatrixTranslation(player.Position.x, 0.0f, player.Position.z));
		return outMatrix;
	}

	static LETransform JoinTransforms(LETransform &a, LETransform &b)
	{
		LETransform connection(XMFLOAT4(0.0f,0.0f,0.0f,0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f));
		connection.Position = Sum(a.Position, b.Position);
		connection.Rotation = Sum(a.Rotation, b.Rotation);
		connection.Scale = Sum(a.Scale, b.Scale);

		return connection;
	}

	//Ignores height
	static float Get2DDistanceBetweenTransforms(LETransform &a, LETransform &b)
	{
		float x, z;
		x = (a.Position.x - b.Position.x);
		z = (a.Position.z - b.Position.z);

		return sqrt(pow(x, 2) + pow(z, 2));
	}
	
};


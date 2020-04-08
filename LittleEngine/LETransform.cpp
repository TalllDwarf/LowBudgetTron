#include "LETransform.h"

LETransform::LETransform(XMFLOAT4 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	Position =  position;
	Rotation =  rotation;
	Scale =  scale;
	FirstRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

LETransform::~LETransform()
{
}

void LETransform::MoveActorLocation(XMFLOAT4 addMovement)
{
	Position.x += addMovement.x;
	Position.y += addMovement.y;
	Position.z += addMovement.z;
}

void LETransform::MoveActorLocation(float x, float y, float z)
{
	Position.x += x;
	Position.y += y;
	Position.z += z;
}
//Creates and returns the world matrix for the transform
XMMATRIX LETransform::GetWorldMatrix()
{
	XMMATRIX worldMatrix = XMMatrixScaling(Scale.x, Scale.y, Scale.z);
	worldMatrix = (worldMatrix * XMMatrixRotationRollPitchYaw(FirstRotation.x, FirstRotation.y, FirstRotation.z));
	worldMatrix = (worldMatrix * XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z));
	worldMatrix = (worldMatrix * XMMatrixTranslationFromVector(XMLoadFloat4(&Position)));

	return worldMatrix;
}

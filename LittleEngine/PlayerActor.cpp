#include "PlayerActor.h"

PlayerActor::PlayerActor() : LEActor(XMFLOAT4(0.0f, 12.0f, -18.0f, 0.0f))
{
	cameraSpeed = 0.0f;
	maxCameraSpeed = 8.0f;
	cameraPitch = 0.0f;
	pitchSpeed = 0.02f;
	cameraYaw = 0.0f;
	yawSpeed = 0.02f;	

	// set starting camera state
	XMFLOAT4 cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	// build view matrix
	XMVECTOR cPos = XMLoadFloat4(&transform.Position);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	XMStoreFloat4x4(&cameraViewMat, XMMatrixLookAtLH(cPos, cTarg, cUp));

	canMove = false;
	renderMesh = false;

	canIncreaseSpeed = true;
}

PlayerActor::~PlayerActor()
{
}

void PlayerActor::Update(float deltaTime)
{
	if (canMove)
	{
		if (canIncreaseSpeed)
		{
			increaseCameraSpeed(deltaTime / 12);
		}

		//Get input
		float forward = cameraSpeed;
		bool right = m_playerInput->KeyPress(VKey_D);
		bool left = m_playerInput->KeyPress(VKey_A);

		if (m_playerInput->KeyPress(VKey_Q))
		{
			increaseCameraSpeed(1.0f);
		}
		else if (m_playerInput->KeyPress(VKey_E))
		{
			increaseCameraSpeed(-1.0f);
		}
		

		if (right)
		{
			cameraYaw += XM_PIDIV2;

			if (cameraYaw > XM_2PI)
			{
				cameraYaw -= XM_2PI;
			}

			trailActor->transform.Rotation.y = cameraYaw;
		}
		else if (left)
		{
			cameraYaw -=  XM_PIDIV2;

			if (cameraYaw < -XM_2PI)
			{
				cameraYaw += XM_2PI;
			}

			trailActor->transform.Rotation.y = cameraYaw;
		}

		transform.Rotation.y = cameraYaw;

		//Camera target
		XMMATRIX cameraRotMat = XMMatrixRotationRollPitchYaw(cameraPitch, cameraYaw, 0);
		XMVECTOR camTarget = XMVector3TransformCoord(DefaultForward, cameraRotMat);
		camTarget = XMVector3Normalize(camTarget);

		//Y rotation
		XMMATRIX rotYTempMat;
		rotYTempMat = XMMatrixRotationY(cameraYaw);

		camRight = XMVector3TransformCoord(DefaultRight, rotYTempMat);
		XMVECTOR camUp = XMVector3TransformCoord(XMLoadFloat4(&cameraUp), rotYTempMat);
		camForward = XMVector3TransformCoord(DefaultForward, rotYTempMat);

		XMVECTOR camPos = XMLoadFloat4(&transform.Position);
		camPos += forward * camForward;

		camTarget = camPos + camTarget;

		XMMATRIX tmpMat = XMMatrixLookAtLH(camPos, camTarget, camUp);
		XMStoreFloat4x4(&cameraViewMat, tmpMat);

		XMStoreFloat4(&cameraUp, camUp);
		XMStoreFloat4(&transform.Position, camPos);
	}
	
	//
	//Camera
	//
	XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat); // load view matrix

	//Transpose for graphic card
	XMStoreFloat4x4(&m_cbCameraObject->viewMatrix, XMMatrixTranspose(viewMat));
	
	m_cbCameraObject->cameraPosition = transform.Position;
}

XMFLOAT3 PlayerActor::GetForwardVector()
{
	XMFLOAT3 forwardVec;
	XMStoreFloat3(&forwardVec, XMVector3Normalize(camForward));

	return forwardVec;
}

void PlayerActor::SetToOthoView()
{
	// build projection and view matrix
	XMMATRIX tmpMat = XMMatrixOrthographicLH(19.2, 10.8, 0.1f, 40.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat); // load projection matrix
	XMStoreFloat4x4(&m_cbCameraObject->projMatrix, XMMatrixTranspose(projMat));
}

void PlayerActor::SetToPerpsView(float width, float height)
{
	//Change projection matrix to perspective
	XMMATRIX projMat = XMMatrixPerspectiveFovLH(80.0f*(3.14f / 180.0f), width / height, 0.1f, 1400.0f);
	XMStoreFloat4x4(&cameraProjMat, projMat);
	XMStoreFloat4x4(&m_cbCameraObject->projMatrix, XMMatrixTranspose(projMat));
}

void PlayerActor::SetBuffer(ConstantCameraBuffer * cbCameraBuffer)
{
	m_cbCameraObject = cbCameraBuffer;
}

void PlayerActor::increaseCameraSpeed(float increaseAmount)
{
	cameraSpeed += increaseAmount;
	canIncreaseSpeed = true;
	if (cameraSpeed > maxCameraSpeed)
	{
		cameraSpeed = maxCameraSpeed;
		canIncreaseSpeed = false;
	}
	else if (cameraSpeed < 0.0f)
	{
		cameraSpeed = 0.0f;
		
	}
}

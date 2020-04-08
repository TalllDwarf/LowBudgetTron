#pragma once
#include "Input.h"
#include "TrailActor.h"

class PlayerActor : public LEActor
{
	//Input
	Input * m_playerInput;

	float cameraSpeed;
	float maxCameraSpeed;

	XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
	XMFLOAT4X4 cameraViewMat; // this will store our view matrix

	XMFLOAT4 cameraUp; // the worlds up vector

	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR camRight;
	XMVECTOR camForward;

	float cameraPitch, cameraYaw;
	float pitchSpeed, yawSpeed;

	ConstantCameraBuffer *m_cbCameraObject;

	bool canMove;
	bool renderMesh;

	std::shared_ptr<TrailActor> trailActor;

public:
	PlayerActor();
	~PlayerActor();

	bool canIncreaseSpeed;

	virtual void Update(float deltaTime) override;

	void SetTrailActor(std::shared_ptr<TrailActor> trail) { trailActor = trail; }

	XMFLOAT3 GetForwardVector();

	//Change camera views
	void SetToOthoView();
	void SetToPerpsView(float width, float height);

	//Can the camera move
	void CanPlayerMove(bool moveable) { canMove = moveable; }

	//Render the players mesh
	void ShowMesh(bool showMesh) { renderMesh = showMesh; }

	//Get the input controller
	void GetInput(Input* playerinput) { m_playerInput = playerinput; }

	//Get the camera buffer giving us better controle
	void SetBuffer(ConstantCameraBuffer* cbCameraBuffer);

	//MAX 8
	bool GoingFast() { return cameraSpeed > 5.5f; }

	float getCameraSpeed() { return cameraSpeed; }
	float getCameraMaxSpeed() { return maxCameraSpeed; }

	void increaseCameraSpeed(float increaseAmount);
};


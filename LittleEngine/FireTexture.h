#pragma once
#include "LEActor.h"

class FireTexture : public LEActor
{
public:
	FireTexture();
	FireTexture(XMFLOAT4 position, XMFLOAT3 rotation);
	~FireTexture();

	//Update function needs to be implemented by all actors
	virtual void Update(float deltaTime);

	//Render the model to the screen
	virtual void Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress);
};


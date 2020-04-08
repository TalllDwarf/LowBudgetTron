#pragma once
#include "LEActor.h"

class LALogo : public LEActor
{
public:
	LALogo();
	~LALogo();

	virtual void Update(float deltaTime) override;

	virtual void Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress);

	virtual void Reset() override;
	
	bool HasFinished() { return finished; }

	void changeTexture(int textureID) { texture = textureID; }

private:

	int texture = 0;

	float delayTimer;
	float endX;

	bool finished;
};


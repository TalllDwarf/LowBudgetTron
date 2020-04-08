#pragma once
#include "LEActor.h"
class TrailActor :
	public LEActor
{
public:
	TrailActor();
	~TrailActor();

	//Render the model to the screen
	virtual void Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress);

	void setNumberToRender(int numberToRender) { numberOfTrails = numberToRender; }

private:

	int numberOfTrails;
};


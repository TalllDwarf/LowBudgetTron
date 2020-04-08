#pragma once
#include "Shader.h"
class BuildingShader :
	public Shader
{
public:
	BuildingShader();
	~BuildingShader();

	//Creates the shader
	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) override;
};


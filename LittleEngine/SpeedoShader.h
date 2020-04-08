#pragma once
#include "Shader.h"
class SpeedoShader :
	public Shader
{
public:
	SpeedoShader();
	~SpeedoShader();

	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) override;
};


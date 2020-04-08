#pragma once
#include "Shader.h"
class FireShader :
	public Shader
{
public:
	FireShader();
	~FireShader();


	//Creates the shader
	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) override;
};


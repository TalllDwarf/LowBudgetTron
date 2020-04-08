#pragma once
#include "Shader.h"
class TransparentTextureShader :
	public Shader
{
public:
	TransparentTextureShader();
	~TransparentTextureShader();

	//Creates the shader
	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) override;

};


#pragma once
#include "Shader.h"
class TransparentColourGlowShader :
	public Shader
{
public:
	TransparentColourGlowShader();
	~TransparentColourGlowShader();

	//Creates the shader
	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) override;

};


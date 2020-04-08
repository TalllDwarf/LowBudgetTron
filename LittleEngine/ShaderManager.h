#pragma once
#include <map>
#include <array>
#include "stdafx.h"
#include "TextureShader.h"
#include "TransparentTextureShader.h"
#include "LightShader.h"
#include "BumpShader.h"
#include "ColourGlowShader.h"
#include "BuildingShader.h"
#include "TransparentColourGlowShader.h"
#include "TrailShader.h"
#include "FireShader.h"
#include "SpeedoShader.h"
#include "TextShader.h"
#include "TestShader.h"

class ShaderManager
{

public:
	ShaderManager();
	~ShaderManager();

	enum ShaderTypes
	{
		//Texture_Shader,
		Transparent_Texture_Shader,
		Light_Texture_Shader,
		Bump_Texture_Shader,
		Colour_Glow_Shader,
		Building_Shader,
		Transparent_Colour_Glow_Shader,
		Trail_Shader,
		Fire_Shader,
		Speedo_Shader,
		Text_Shader,
		Test_Shader
	};

	bool Initialise(ID3D12Device* device, DXGI_SAMPLE_DESC sampleDesc);

	ID3D12PipelineState* GetPipelineState(ShaderTypes type);

	ID3D12RootSignature* GetRootSignature() { return rootSignature; }

	void CleanUp();


private:

	ID3D12RootSignature * rootSignature; // root signature defines data shaders will access

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	std::map<int, std::shared_ptr<Shader>> shaders;

};


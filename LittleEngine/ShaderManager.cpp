#include "ShaderManager.h"

ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

bool ShaderManager::Initialise(ID3D12Device * device, DXGI_SAMPLE_DESC sampleDesc)
{
	
	// -- Create root signiture -- //
	HRESULT hr;

	int srvCount = SRV_DESCRIPTOR_COUNT;

	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, srvCount, 0);

	CD3DX12_ROOT_PARAMETER rootP[4];

	rootP[0].InitAsConstantBufferView(0);
	rootP[1].InitAsConstantBufferView(1);
	rootP[2].InitAsConstantBufferView(2);
	rootP[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
		_countof(rootP),
		rootP,
		(UINT)staticSamplers.size(),
		staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
	{
		return false;
	}

	/*
	//
	//Texture Shader
	//
	
	{
		std::shared_ptr<TextureShader> texShader = std::make_shared<TextureShader>();

		if (!texShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Texture_Shader] = texShader;
	}
	*/
	
	//
	//Transparent Texture Shader
	//
	{
		std::shared_ptr<TransparentTextureShader> tTexShader = std::make_shared<TransparentTextureShader>();

		if (!tTexShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Transparent_Texture_Shader] = tTexShader;
	}
	

	//
	//Light Texture
	//
	{
		std::shared_ptr<LightShader> lightShader = std::make_shared<LightShader>();

		if (!lightShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Light_Texture_Shader] = lightShader;
	}

	//
	//Bump Texture
	//
	{
		std::shared_ptr<BumpShader> bumpShader = std::make_shared<BumpShader>();

		if (!bumpShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Bump_Texture_Shader] = bumpShader;
	}

	//
	//Colour glow shader
	//
	{
		std::shared_ptr<ColourGlowShader> glowShader = std::make_shared<ColourGlowShader>();

		if (!glowShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Colour_Glow_Shader] = glowShader;
	}

	//
	//Building Shader
	//
	{
		std::shared_ptr<BuildingShader> buildingShader = std::make_shared<BuildingShader>();

		if (!buildingShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Building_Shader] = buildingShader;
	}

	//
	//Colour glow Shader W transparency
	//
	{
		std::shared_ptr<TransparentColourGlowShader> transColourShader = std::make_shared<TransparentColourGlowShader>();

		if (!transColourShader->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Transparent_Colour_Glow_Shader] = transColourShader;
	}

	//
	//Trail Shader
	//
	{
		std::shared_ptr<TrailShader> trail = std::make_shared<TrailShader>();

		if (!trail->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Trail_Shader] = trail;

	}

	//
	//Fire Shader
	//
	{
		std::shared_ptr<FireShader> fire = std::make_shared<FireShader>();

		if (!fire->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Fire_Shader] = fire;
	}

	//Speedo shader
	{
		std::shared_ptr<SpeedoShader> speedo = std::make_shared<SpeedoShader>();

		if (!speedo->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Speedo_Shader] = speedo;
	}

	//Text shader
	{
		std::shared_ptr<TextShader> text = std::make_shared<TextShader>();

		if (!text->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Text_Shader] = text;

	}
	
	//
	//Test Shader
	//
	{
		std::shared_ptr<TestShader> test = std::make_shared<TestShader>();

		if (!test->Initialise(device, rootSignature, sampleDesc))
		{
			return false;
		}

		shaders[ShaderTypes::Test_Shader] = test;
	}

	return true;
}

ID3D12PipelineState * ShaderManager::GetPipelineState(ShaderTypes type)
{
	return shaders[type]->GetPipelineState();
}

void ShaderManager::CleanUp()
{
	SAFE_RELEASE(rootSignature);
}


//TODO: Implement multiple static samplers
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> ShaderManager::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers. So just define them 

	// all up front and keep them available as part of the root signature. 

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(

		0, // shaderRegister

		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(

		1, // shaderRegister

		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW


	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(

		2, // shaderRegister

		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(

		3, // shaderRegister

		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(

		4, // shaderRegister

		D3D12_FILTER_ANISOTROPIC, // filter

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressW

		0.0f,               // mipLODBias

		8);                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(

		5, // shaderRegister

		D3D12_FILTER_ANISOTROPIC, // filter

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV

		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressW

		0.0f,               // mipLODBias

		8);                // maxAnisotropy

	return {
		pointWrap, pointClamp, 
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp};
	
}

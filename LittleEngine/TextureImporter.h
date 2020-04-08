#pragma once
#include "stdafx.h"

class TextureImporter
{
public:
	TextureImporter();
	~TextureImporter();

	//Gets the device and command list then loads all of the models
	void Initialise(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	//Loads the texture file
	bool LoadTexture(const wchar_t* texturePath);

	//Clean up textures
	void TextureCleanup();

	DXGI_FORMAT GetTextureFormat();

	ID3D12Resource* GetTextureBuffer();

private:

	ID3D12Device * m_device;
	ID3D12GraphicsCommandList* m_commandList;

	UINT descriptorSize;

	bool hasBump;

	//Image data
	BYTE * imageData;

	D3D12_RESOURCE_DESC textureDesc;

	ID3D12Resource* m_textureBuffer;
	ID3D12Resource* m_textureBufferUploadHeap;
};


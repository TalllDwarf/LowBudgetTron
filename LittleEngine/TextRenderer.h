#pragma once
#include "ShaderManager.h"

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	bool Initialise(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, float width, float height);


	Font LoadFont(LPCWSTR filename, int windowWidth, int windowHeight);

	void RenderText(ID3D12DescriptorHeap* dsDescriptorHeap, std::shared_ptr<ShaderManager> shaderManger, std::wstring text, int usedCharCount, XMFLOAT2 pos, int frameIndex, XMFLOAT2 scale = XMFLOAT2(1.0f, 1.0f), XMFLOAT2 padding = XMFLOAT2(0.5f, 0.5f), XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

private:
	Font font; //this will store our font
	int maxNumTexChar = 1024;
	UINT srvHandleSize;
	ID3D12DescriptorHeap* mainTextDescriptorHeap;

	ID3D12Resource* textVertexBuffer[FRAMEBUFFERCOUNT];
	D3D12_VERTEX_BUFFER_VIEW textVertexBufferView[FRAMEBUFFERCOUNT];

	UINT8* textVBGPUAddress[FRAMEBUFFERCOUNT];

	ID3D12Device * m_device;
	ID3D12GraphicsCommandList* m_commandList;
};


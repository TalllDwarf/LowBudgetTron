#include "FireTexture.h"

FireTexture::FireTexture() : LEActor()
{
}

FireTexture::FireTexture(XMFLOAT4 position, XMFLOAT3 rotation) : LEActor(position, rotation)
{
}


FireTexture::~FireTexture()
{
}

void FireTexture::Update(float deltaTime)
{
}

void FireTexture::Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress)
{
	m_commandList->SetGraphicsRootConstantBufferView(2, GPUVirtualAdress + ConstantBufferOffset);

	if (numberOfImages > 0)
	{
		ID3D12DescriptorHeap* pDescriptorHeaps[] = { m_textureDescriptorHeap };
		m_commandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

		//texture->SetDescriptorHeap(textureRootParameterIndex);
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//m_commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(3, tex);
	}

	model->SetBuffers();
	m_commandList->DrawIndexedInstanced(model->GetIndeciesCount(), 1, 0, 0, 0);
}
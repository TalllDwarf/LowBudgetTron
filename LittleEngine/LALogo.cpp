#include "LALogo.h"

LALogo::LALogo() : LEActor(XMFLOAT4(17.0f,0.0f,0.0f,0.0f))
{
	Velocity.x = 10.0f;
	delayTimer =2.5f;
	endX = -17;
	finished = false;
}

LALogo::~LALogo()
{
}

void LALogo::Update(float deltaTime)
{
	if (transform.Position.x > 0)
	{
		transform.Position.x -= (Velocity.x * deltaTime);
	}
	else if(delayTimer > 0)
	{
		delayTimer -= deltaTime;
	}
	else
	{
		transform.Position.x -= (Velocity.x * deltaTime);
		if (transform.Position.x < endX)
		{
			finished = true;
		}
	}
}

void LALogo::Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress)
{
	// set the actors's constant buffer
	m_commandList->SetGraphicsRootConstantBufferView(2, GPUVirtualAdress + ConstantBufferOffset);

	if (numberOfImages > 0)
	{
		ID3D12DescriptorHeap* pDescriptorHeaps[] = { m_textureDescriptorHeap };
		m_commandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

		//texture->SetDescriptorHeap(textureRootParameterIndex);
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), texture, rtvIncrementSize);

		//m_commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		m_commandList->SetGraphicsRootDescriptorTable(textureRootParameterIndex, tex);
	}

	//Set model then draw it 
	if (model)
	{
		model->SetBuffers();

		// draw actor
		m_commandList->DrawIndexedInstanced(model->GetIndeciesCount(), 1, 0, 0, 0);
	}
}

void LALogo::Reset()
{
	transform.Position.x = 17.0f;
	delayTimer = 2.5f;
	finished = false;
}

#include "TrailActor.h"



TrailActor::TrailActor()
{
	numberOfTrails = 0;
}


TrailActor::~TrailActor()
{
}

void TrailActor::Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress)
{
	if (numberOfTrails > 0)
	{

		// set the actors's constant buffer
		m_commandList->SetGraphicsRootConstantBufferView(2, GPUVirtualAdress + ConstantBufferOffset);

		if (numberOfImages > 0)
		{
			ID3D12DescriptorHeap* pDescriptorHeaps[] = { m_textureDescriptorHeap };
			m_commandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);

			//texture->SetDescriptorHeap(textureRootParameterIndex);
			CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

			//m_commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_textureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			m_commandList->SetGraphicsRootDescriptorTable(textureRootParameterIndex, tex);
		}

		//Set model then draw it 
		if (model)
		{
			model->SetBuffers();

			// draw actor
			m_commandList->DrawIndexedInstanced(model->GetIndeciesCount(), numberOfTrails, 0, 0, 0);
		}
	}
}

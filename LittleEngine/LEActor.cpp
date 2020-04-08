#include "LEActor.h"

LEActor::LEActor() : transform(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f)), Velocity(XMFLOAT3(0.0f,0.0f,0.0f))
{}

LEActor::LEActor(XMFLOAT4 startPosition) : transform(startPosition, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f)), Velocity(XMFLOAT3(0.0f, 0.0f, 0.0f))
{}

LEActor::LEActor(XMFLOAT4 startPosition, XMFLOAT3 startRotation) : transform(startPosition, startRotation, XMFLOAT3(1.0f,1.0f,1.0f)), Velocity(XMFLOAT3(0.0f, 0.0f, 0.0f))
{}

LEActor::LEActor(XMFLOAT4 startPosition, XMFLOAT3 startRotation, XMFLOAT3 startScale) :transform(startPosition,startRotation,startScale), Velocity(XMFLOAT3(0.0f, 0.0f, 0.0f))
{}

LEActor::~LEActor()
{}

bool LEActor::Initialise(ID3D12Device * device, ID3D12GraphicsCommandList * commandList, std::shared_ptr<ModelImporter> actorModel, int numOfImages...)
{
	m_device = device;
	m_commandList = commandList;
	model = actorModel;
	numberOfImages = numOfImages;

	updateMove = false;
	updateRotate = false;

	rtvIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (numOfImages != 0)
	{
		HRESULT hr;

		int numOfDesc = SRV_DESCRIPTOR_COUNT;

		//create the descriptor heap that will store our srv
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = numOfDesc;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_textureDescriptorHeap));
		if (FAILED(hr))
		{
			return false;
		}

		descriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//Get a pointer to the start of the heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		//Now we create a shader resource view (descriptor that points to the texture and describes it)
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		//
		//Add all the textures to our descriptor
		//
		va_list args;
		va_start(args, numOfImages);

		for (int i = 0; i < numOfImages; i++)
		{
			std::shared_ptr<TextureImporter> texture = va_arg(args, std::shared_ptr<TextureImporter>);

			srvDesc.Format = texture->GetTextureFormat();

			m_device->CreateShaderResourceView(texture->GetTextureBuffer(), &srvDesc, hDescriptor);

			if(numOfImages-1 != i)
				hDescriptor.Offset(1, descriptorSize);
		}

		va_end(args);
	}

	return true;
}


void LEActor::InitialiseAABBCollision()
{
	collisionAABB = std::make_shared<CollisionAABB>();
	collisionAABB->CreateAABB(model);
}

void LEActor::InitialiseSphereCollision()
{
	collisionSphere = std::make_shared<CollisionSphere>();
	collisionSphere->CreateSphereCollider(model);
}

void LEActor::UpdateCollision(XMMATRIX& worldMatrix)
{
	if (collisionAABB != nullptr)
	{
		collisionAABB->CalculateWorldAABB(worldMatrix);
	}

	if (collisionSphere != nullptr)
	{
		//TODO: Sphere Collision
	}
}

void LEActor::Update(float deltaTime)
{
	if (updateRotate)
	{
		transform.Rotation = Sum(transform.Rotation, UpdateRotation);
	}
}

void LEActor::Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress)
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
		m_commandList->DrawIndexedInstanced(model->GetIndeciesCount(), 1, 0, 0, 0);
	}

}
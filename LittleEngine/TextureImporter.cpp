
#include "TextureImporter.h"



TextureImporter::TextureImporter()
{
	hasBump = false;
}


TextureImporter::~TextureImporter()
{
}

void TextureImporter::Initialise(ID3D12Device * device, ID3D12GraphicsCommandList * commandList)
{
	m_device = device;
	m_commandList = commandList;
}

bool TextureImporter::LoadTexture(const wchar_t * texturePath)
{
	HRESULT hr;

	// -- Load image -- //
	//D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, texturePath, imageBytesPerRow);

	//make sure we have data
	if (imageSize <= 0)
	{
		return false;
	}

	//Create a defualt heap where the upload heap will copy its texture into 
	hr = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_textureBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	m_textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	m_device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	//now we create an upload heap to upload our texture to the gpu
	hr = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //upload heap
		D3D12_HEAP_FLAG_NONE, // no flag
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize),//Resource description for a buffer (storing the image data in the heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ,//We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&m_textureBufferUploadHeap));

	if (FAILED(hr))
	{
		return false;
	}

	m_textureBufferUploadHeap->SetName(L"Texture buffer upload resource heap");

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &imageData[0]; //pointer to our image data
	textureData.RowPitch = imageBytesPerRow; //Size of all our triangle vertex data
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; //also the size of all out triangle vertex data

																	//Now we copy the upload buffer contents to the default heap
	UpdateSubresources(m_commandList, m_textureBuffer, m_textureBufferUploadHeap, 0, 0, 1, &textureData);

	//transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	/*
	
	//create the descriptor heap that will store our srv
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2;
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
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	*/


	//m_device->CreateShaderResourceView(m_textureBuffer, &srvDesc, hDescriptor);
	//hDescriptor.Offset(1, descriptorSize);
	//m_device->CreateShaderResourceView(m_textureBuffer, &srvDesc, m_textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	return true;
}

void TextureImporter::TextureCleanup()
{
	delete imageData;
}

DXGI_FORMAT TextureImporter::GetTextureFormat()
{
	return textureDesc.Format;
}

ID3D12Resource * TextureImporter::GetTextureBuffer()
{
	return m_textureBuffer;
}

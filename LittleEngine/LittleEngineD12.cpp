#include "LittleEngineD12.h"


LittleEngineD12::LittleEngineD12()
{
	LEState = State::Logo;
	runningSin = 0;
}


LittleEngineD12::~LittleEngineD12()
{
}


//
//Player Killed
//
void LittleEngineD12::PlayerLost()
{
	LEState = State::Lost;
	APlayer->CanPlayerMove(false);
}

void LittleEngineD12::ResetGame()
{
	//Remove the trail from the map
	invunerableTimer = 2.0f;
	ZeroMemory(&trailInstances, sizeof(trailInstances));
	currentTrail = 0;
	createTrail = true;
	trailMiddle->setNumberToRender(0);
	trailMiddle->transform.Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	APlayer->transform.Position = XMFLOAT4(0.0f, 12.0f, -18.0f, 1.0f);

	APlayer->increaseCameraSpeed(-1000);
	APlayer->CanPlayerMove(true);

	timeAlive = 0.0f;

	LEState = State::Playing;
}

bool LittleEngineD12::InitD3D(HWND hwnd, int width, int height, bool fullScreen, Input* playerInput)
{
	m_playerInput = playerInput;

	HRESULT hr;

	windowSize.x = width;
	windowSize.y = height;

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	IDXGIAdapter1* adapter; // adapters are the graphic card (Including motherboard imbeded graphics)

	int adapterIndex = 0; // we'll start looking for directX12 compatible deivces starting at 0

	bool adapterFound = false;

	//Find the first hardware GPU that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		//check if it is software device
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			//We do not want software devices
			adapterIndex++;
			continue;
		}

		//We want a device that is compatible with direct3d 12 (Feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);

		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		return false;
	}

	//Creates our device
	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	// -- Create the command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {}; // Use the default values
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));

	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	// -- Create the swap chain (tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // Descripes the display mode
	backBufferDesc.Width = width; //Buffer width
	backBufferDesc.Height = height; // Buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //format the buffer RGBA 32 bits, 8 bits per chanel

														//Describe our multi-sampling. We are not multi-sampling so just 1
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	//Describe and create the swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FRAMEBUFFERCOUNT; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; //our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // This says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //DXGI will discard the buffer data after we call present
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc = sampleDesc; // our multisample description
	swapChainDesc.Windowed = !fullScreen; // set to true, then if in fullscreen must call SetFullScreenState with ture for fullscreen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &tempSwapChain);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// -- Create the back buffers (render target view) Descriptor heap -- //

	//Describe the RTV descriptor heap and create
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAMEBUFFERCOUNT; // number of descriptos for this heap
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //this heap is a Render Targer View heap

													   //This heap will not be directly referenced by the shaders (not visible shaders), as this will store the output from the pipeline
													   //otherwise we would set the heap's flag to D3D12_Descriptor_Heap_Flag_Shader_Visible
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));

	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	//get the size of the descriptor in this heap (this is just a rtv heap, so only rtv descriptors should be stored in it.
	//descripor sizes my vary from device to device. which is why there are no set size and we must ask the devvice to give us the size
	//We will use this size to increment a descripto handle offset
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//Get a handle to the first descriptor in this heap, a handle is basically a pointer,
	//but we cannot literally use it like a c++ pointer
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//Create an RTV for each buffer
	for (int i = 0; i < FRAMEBUFFERCOUNT; i++)
	{
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr))
		{
			LEState = State::Stopping;
			return false;
		}

		//we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
		device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

		//We increment the rtv handle by the rtv descriptor size we got above
		rtvHandle.Offset(1, rtvDescriptorSize);

		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));

		if (FAILED(hr))
		{
			LEState = State::Stopping;
			return false;
		}

	}

	//Create the command list with the first allocator
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	//Command list are created in the recording state our main loop will set this up so we can close it here
	//commandList->Close();

	// -- Creat a Fence and Fence events -- //

	//Create the fences
	for (int i = 0; i < FRAMEBUFFERCOUNT; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			LEState = State::Stopping;
			return false;
		}

		fenceValue[i] = 0;
	}

	//Create a handle to a fence event
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	//Loads and stores the shaders that we need
	shaderManager = std::make_shared<ShaderManager>();
	if (!shaderManager->Initialise(device, sampleDesc))
	{
		return false;
	}

	//Text
	textRenderer = std::make_shared<TextRenderer>();
	if (!textRenderer->Initialise(device, commandList, width, height))
	{
		return false;
	}


	//Loads and stores all the models we need
	modelManager = std::make_shared<ModelManager>();
	if (!modelManager->Initialise(device, commandList))
	{
		return false;
	}


	//Loads and stores all the texutres we need
	textureManager = std::make_shared<TextureManager>();

	if (!textureManager->Initialise(device, commandList))
	{
		return false;
	}

	collision = std::make_shared<PCollision>();





	//
	//
	//Dynamic Actor init
	//

	//
	//Player
	//
	APlayer = std::make_shared<PlayerActor>();
	//The player does not have a model or texture
	APlayer->Initialise(device, commandList, nullptr, 0);

	//Player Mesh neon
	//Uses a different shader so we have it seperate
	playerNeon = std::make_shared<LEActor>();
	playerNeon->Initialise(device, commandList, modelManager->GetModel(Model::BIKE_NEON), 1, textureManager->GetTexture(Texture::METAL_5_LIGHT));

	//
	//Player mesh
	//

	{
		std::shared_ptr<LEActor> body = std::make_shared<LEActor>();
		body->Initialise(device, commandList, modelManager->GetModel(Model::BIKE),
			3,
			textureManager->GetTexture(Texture::BIKE_BASE),
			textureManager->GetTexture(Texture::METAL_5_NORMAL),
			textureManager->GetTexture(Texture::METAL_5_SPEC));

		playerMesh.push_back(body);
	}

	{
		std::shared_ptr<LEActor> wheelFront = std::make_shared<LEActor>(XMFLOAT4(0.0f, 4.0f, 10.0f, 1.0f), XMFLOAT3(), XMFLOAT3(1.02f, 1.06f, 1.06f));
		wheelFront->Initialise(device, commandList, modelManager->GetModel(Model::BIKE_WHEEL),
			3,
			textureManager->GetTexture(Texture::BIKE_TIRE),
			textureManager->GetTexture(Texture::METAL_5_NORMAL),
			textureManager->GetTexture(Texture::METAL_5_SPEC));

		wheelFront->InitialiseAABBCollision();

		wheelFront->SetConstantRotation(XMFLOAT3(0.0f, 0.0f, 0.1f));

		playerCollider = wheelFront;
		playerMesh.push_back(wheelFront);
	}

	//
	//Speedo
	//
	speedo = std::make_shared<LEActor>(XMFLOAT4(0.0f, 10.0f, 6.0f, 1.0f), XMFLOAT3(0.0f, XM_PI, XM_PI / 3));
	speedo->Initialise(device, commandList, modelManager->GetModel(Model::BIKE_SPEEDO),
		2,
		textureManager->GetTexture(Texture::BIKE_SPEEDO),
		textureManager->GetTexture(Texture::METAL_5_LIGHT));


	speedoNeedle = std::make_shared<LEActor>(XMFLOAT4(0.0f, 10.0f, 6.0f, 1.0f), XMFLOAT3(0.0f, XM_PI, XM_PI / 3));
	speedoNeedle->Initialise(device, commandList, modelManager->GetModel(Model::BIKE_SPEEDO_NEEDLE),
		2,
		textureManager->GetTexture(Texture::FIRE),
		textureManager->GetTexture(Texture::METAL_5_LIGHT));


	//
	//Trail start
	//
	/*
	{
		std::shared_ptr<LEActor> trailActor = std::make_shared<LEActor>();

		trailActor->Initialise(device, commandList, modelManager->GetModel(Model::TRAIL_START),
			3,
			textureManager->GetTexture(Texture::TRAIL_1_COLOR),
			textureManager->GetTexture(Texture::TRAIL_2_COLOR),
			textureManager->GetTexture(Texture::TRAIL_1_LIGHT));
		trailActor->InitialiseAABBCollision();

		trailStart = trailActor;
	}
	*/

	//Spawn point of the trail
	spawnPoint = std::make_shared<LEActor>();
	spawnPoint->transform.Position.z = -10.0f;

	{
		std::shared_ptr<TrailActor> trailActor = std::make_shared <TrailActor> ();
		trailActor->Initialise(device, commandList, modelManager->GetModel(Model::TRAIL_MIDDLE),
			3,
			textureManager->GetTexture(Texture::TRAIL_1_COLOR),
			textureManager->GetTexture(Texture::TRAIL_2_COLOR),
			textureManager->GetTexture(Texture::TRAIL_1_LIGHT));
		trailActor->InitialiseAABBCollision();

		//Prepare trailInstances for gpu 
		trailMiddle = trailActor;
		APlayer->SetTrailActor(trailActor);
	}


	//
	//Fire bike
	//

	{
		std::shared_ptr<FireTexture> fireActor = std::make_shared<FireTexture>();
		fireActor->transform.Scale = XMFLOAT3(1.1f, 1.1f, 1.1f);
		fireActor->Initialise(device, commandList, modelManager->GetModel(Model::BIKE),
			3,
			textureManager->GetTexture(Texture::FIRE),
			textureManager->GetTexture(Texture::FIRE_NOISE),
			textureManager->GetTexture(Texture::FIRE_ALPHA));

		fireObject = fireActor;
	}

	//
	//Fire corners
	//

	{
		std::shared_ptr<FireTexture> fireWallActor = std::make_shared<FireTexture>();
		fireWallActor->transform.Position = XMFLOAT4(-500.5f, 110.0f, 500.0f, 0.0f);
		fireWallActor->transform.Rotation = XMFLOAT3(0.0f, ((7 * XM_PI) / 4), 0.0f);
		fireWallActor->transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0);
		fireWallActor->Initialise(device, commandList, modelManager->GetModel(Model::FIRE_WALL), 3,
			textureManager->GetTexture(Texture::FIRE),
			textureManager->GetTexture(Texture::FIRE_NOISE),
			textureManager->GetTexture(Texture::FIRE_BIG_ALPHA));

		fireList.push_back(fireWallActor);
	}

	{
		std::shared_ptr<FireTexture> fireWallActor = std::make_shared<FireTexture>();
		fireWallActor->transform.Position = XMFLOAT4(500.5f, 110.0f, -500.0f, 0.0f);
		fireWallActor->transform.Rotation = XMFLOAT3(0.0f, ((3 * XM_PI) / 4), 0.0f);
		fireWallActor->transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0);
		fireWallActor->Initialise(device, commandList, modelManager->GetModel(Model::FIRE_WALL), 3,
			textureManager->GetTexture(Texture::FIRE),
			textureManager->GetTexture(Texture::FIRE_NOISE),
			textureManager->GetTexture(Texture::FIRE_BIG_ALPHA));

		fireList.push_back(fireWallActor);
	}

	{
		std::shared_ptr<FireTexture> fireWallActor = std::make_shared<FireTexture>();
		fireWallActor->transform.Position = XMFLOAT4(-500.5f, 110.0f, -500.0f, 0.0f);
		fireWallActor->transform.Rotation = XMFLOAT3(0.0f, ((5 * XM_PI) / 4), 0.0f);
		fireWallActor->transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0);
		fireWallActor->Initialise(device, commandList, modelManager->GetModel(Model::FIRE_WALL), 3,
			textureManager->GetTexture(Texture::FIRE),
			textureManager->GetTexture(Texture::FIRE_NOISE),
			textureManager->GetTexture(Texture::FIRE_BIG_ALPHA));

		fireList.push_back(fireWallActor);
	}

	{
		std::shared_ptr<FireTexture> fireWallActor = std::make_shared<FireTexture>();
		fireWallActor->transform.Position = XMFLOAT4(500.5f, 110.0f, 500.0f, 0.0f);
		fireWallActor->transform.Rotation = XMFLOAT3(0.0f, (XM_PI / 4), 0.0f);
		fireWallActor->transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0);
		fireWallActor->Initialise(device, commandList, modelManager->GetModel(Model::FIRE_WALL), 3,
			textureManager->GetTexture(Texture::FIRE),
			textureManager->GetTexture(Texture::FIRE_NOISE),
			textureManager->GetTexture(Texture::FIRE_BIG_ALPHA));

		fireList.push_back(fireWallActor);
	}

	//
	//Logo
	//

	ALogo = std::make_shared<LALogo>();
	ALogo->Initialise(device, commandList, modelManager->GetModel(Model::PLANE), 
		1,
		textureManager->GetTexture(Texture::LOGO));

	gameLogo = std::make_shared<LEActor>(XMFLOAT4(0.0f,1.0f,0.0f,0.0f));
	gameLogo->Initialise(device, commandList, modelManager->GetModel(Model::PLANE),
		1,
		textureManager->GetTexture(Texture::TRON));

	//
	//
	//Dynamic actor init
	//
	//



	//
	//
	//Static actor init
	//
	//

	//FrontBack Walls
	//if they all share the same texture we do not need to assign each one the texture
	//The texture will stay for each one
	{
		std::shared_ptr<LEActor> wallActor = std::make_shared<LEActor>(XMFLOAT4(500.0f, 0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, ((3 * XM_PI) / 2), 0.0f));
		wallActor->Initialise(device, commandList, modelManager->GetModel(Model::WALL),
			1, //Number of textures we are going to use
			textureManager->GetTexture(Texture::METAL_5_LIGHT));

		wallActor->InitialiseAABBCollision();

		collidableMapActorList.push_back(wallActor);
	}

	{
		std::shared_ptr<LEActor> wallActor = std::make_shared<LEActor>(XMFLOAT4(-500.0f, 0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, (XM_PI / 2), 0.0f));
		wallActor->Initialise(device, commandList, modelManager->GetModel(Model::WALL), 0);
		wallActor->InitialiseAABBCollision();

		collidableMapActorList.push_back(wallActor);
	}

	//Side Walls

	{
		std::shared_ptr<LEActor> wallActor = std::make_shared<LEActor>(XMFLOAT4(0.0f, 0.0f, 500.0f, 0.0f), XMFLOAT3(0.0f, XM_PI, 0.0f));
		wallActor->Initialise(device, commandList, modelManager->GetModel(Model::WALL), 0);
		wallActor->InitialiseAABBCollision();

		collidableMapActorList.push_back(wallActor);
	}

	{
		std::shared_ptr<LEActor> wallActor = std::make_shared<LEActor>(XMFLOAT4(0.0f, 0.0f, -500.0f, 0.0f));
		wallActor->Initialise(device, commandList, modelManager->GetModel(Model::WALL), 0);
		wallActor->InitialiseAABBCollision();

		collidableMapActorList.push_back(wallActor);
	}

	//
	//Sides
	//
	{
		std::shared_ptr<LEActor> mapSides = std::make_shared<LEActor>(XMFLOAT4(0.0f, 110.0f, 0.0f, 0.0f), XMFLOAT3(), XMFLOAT3(1.1f, 1.1f, 1.1f));
		mapSides->Initialise(device, commandList, modelManager->GetModel(Model::SIDES),
			3, //Number of textures we are going to use
			textureManager->GetTexture(Texture::FLOOR_1_COLOR),
			textureManager->GetTexture(Texture::METAL_5_LIGHT),
			textureManager->GetTexture(Texture::TRAIL_2_COLOR));

		mapActorList.push_back(mapSides);
	}
	//
	//Roof
	//
	{
		std::shared_ptr<LEActor> mapRoof = std::make_shared<LEActor>(XMFLOAT4(0.0f, 110.0f, 0.0f, 0.0f), XMFLOAT3(), XMFLOAT3(1.1f, 1.1f, 1.1f));
		mapRoof->Initialise(device, commandList, modelManager->GetModel(Model::ROOF), 0);

		mapActorList.push_back(mapRoof);
	}

	//Floor
	{
		std::shared_ptr<LEActor> floorActor = std::make_shared<LEActor>();
		floorActor->Initialise(device, commandList, modelManager->GetModel(Model::FLOOR),
			3, //Number of textures we are going to use
			textureManager->GetTexture(Texture::FLOOR_1_COLOR), //The 4 textures 
			textureManager->GetTexture(Texture::FLOOR_1_LIGHT),
			textureManager->GetTexture(Texture::METAL_5_COLOR));

		mapActorList.push_back(floorActor);
	}



	//
	//
	//End Actor Init
	//
	//





	// Depth/Stencil testing
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dSDescriptorHeap));
	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0.0f;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthStencilBuffer));
	dSDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, dSDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// create the constant buffer resource heap
	// We will update the constant buffer one or more times per frame, so we will use only an upload heap
	// unlike previously we used an upload heap to upload the vertex and index data, and then copied over
	// to a default heap. If you plan to use a resource for more than a couple frames, it is usually more
	// efficient to copy to a default heap where it stays on the gpu. In this case, our constant buffer
	// will be modified and uploaded at least once per frame, so we only use an upload heap

	// first we will create a resource heap (upload heap) for each frame for the cubes constant buffers
	// As you can see, we are allocating 64KB for each resource we create. Buffer resource heaps must be
	// an alignment of 64KB. We are creating 3 resources, one for each frame. Each constant buffer is 
	// only a 4x4 matrix of floats in this tutorial. So with a float being 4 bytes, we have 
	// 16 floats in one constant buffer, and we will store 2 constant buffers in each
	// heap, one for each cube, thats only 64x2 bits, or 128 bits we are using for each
	// resource, and each resource must be at least 64KB (65536 bits)
	for (int i = 0; i < FRAMEBUFFERCOUNT; i++)
	{
		//Create a resource for cube 1
		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, //No flags
			&CD3DX12_RESOURCE_DESC::Buffer((1024 * 64)*2), //Size of the resource heap, Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, //will be data that is read so we keep it in the generic read state 
			nullptr, //We do not use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeaps[i]));
		constantBufferUploadHeaps[i]->SetName(L"Constant buffer upload resource heap");

		ZeroMemory(&cbPerObject, sizeof(cbPerObject));
		ZeroMemory(&cbCameraObject, sizeof(cbCameraObject));
		ZeroMemory(&cbLightObject, sizeof(cbLightObject));
		ZeroMemory(&cbFireObject, sizeof(cbFireObject));
		ZeroMemory(&distortionObject, sizeof(distortionObject));
		ZeroMemory(&trailInstances, sizeof(trailInstances));

		CD3DX12_RANGE readRange(0, 0); // we do not intent to read from this resource from the cpu so we set end to less than or equal to begin
		hr = constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));

		//Position of the buffer
		//Use this to keep the next position to store the buffer, helping reduce make mistakes
		UINT position = 0;

		// Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
		// so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
		memcpy(cbvGPUAddress[i], &cbCameraObject, sizeof(cbCameraObject)); // Camera constant buffer
		position += ConstantCameraObjectAlignedSize;

		//Set our light buffer
		memcpy(cbvGPUAddress[i] + position, &cbLightObject, sizeof(cbLightObject));
		position += ConstantLightObjectAlignedSize;

		//Give the actor there constant buffer location
		ALogo->SetConstantBufferOffset(position);
		gameLogo->SetConstantBufferOffset(position);

		//Shares location with the logo one one will use it at a time
		//Player mesh neon
		playerNeon->SetConstantBufferOffset(position);
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(playerNeon->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));

		position += ConstantBufferObjectAlignedSize;

		//Speedo
		speedo->SetConstantBufferOffset(position);
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(speedo->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));
		position += ConstantBufferObjectAlignedSize;

		speedoNeedle->SetConstantBufferOffset(position);
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(speedoNeedle->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));
		position += ConstantBufferObjectAlignedSize;


		//Tell each actor its constant buffer location
		for (std::vector<std::shared_ptr<LEActor>>::iterator it = playerMesh.begin(); it != playerMesh.end(); it++)
		{
			(*it)->SetConstantBufferOffset(position);

			XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose((*it)->transform.GetWorldMatrix()));
			memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));

			position += ConstantBufferObjectAlignedSize;
		}


		//Tell each actor its constant buffer location
		for (std::vector<std::shared_ptr<LEActor>>::iterator it = mapActorList.begin(); it != mapActorList.end(); it++)
		{
			(*it)->SetConstantBufferOffset(position);

			XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose((*it)->transform.GetWorldMatrix()));
			memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));

			position += ConstantBufferObjectAlignedSize;
		}

		for (std::vector<std::shared_ptr<LEActor>>::iterator it = collidableMapActorList.begin(); it != collidableMapActorList.end(); it++)
		{
			//Set constant buffer
			(*it)->SetConstantBufferOffset(position);

			//write the world matrix into the buffer
			XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose((*it)->transform.GetWorldMatrix()));
			memcpy(cbvGPUAddress[i] + position, &cbPerObject, sizeof(cbPerObject));

			position += ConstantBufferObjectAlignedSize;
		}

		for (std::vector<std::shared_ptr<FireTexture>>::iterator it = fireList.begin(); it != fireList.end(); it++)
		{
			(*it)->SetConstantBufferOffset(position);

			XMStoreFloat4x4(&cbFireObject.worldMatrix, XMMatrixTranspose((*it)->transform.GetWorldMatrix()));
			memcpy(cbvGPUAddress[i] + position, &cbFireObject, sizeof(cbFireObject));

			position += ConstantFireObjectAlignedSize;
		}

		//Fire constant
		fireObject->SetConstantBufferOffset(position);
		XMStoreFloat4x4(&cbFireObject.worldMatrix, XMMatrixTranspose(fireObject->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[i] + position, &cbFireObject, sizeof(cbFireObject));

		position += ConstantFireObjectAlignedSize;

		//Fire distortian
		memcpy(cbvGPUAddress[i] + position, &distortionObject, sizeof(distortionObject));
		distortionLocation = position;

		position += ConstantDistortionAlignedSize;

		//Trail
		trailMiddle->SetConstantBufferOffset(position);

		memcpy(cbvGPUAddress[i] + position, &trailInstances, sizeof(trailInstances));
	}

	// Now we execute the command list to upload the initial assets (triangle data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		LEState = State::Stopping;
		return false;
	}

	//Texture Cleanup
	textureManager->TextureCleanup();

	// Fill out the Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;


	//Camera setup
	APlayer->SetBuffer(&cbCameraObject);
	APlayer->SetToOthoView();
	APlayer->GetInput(playerInput);

	//
	//Light Buffer
	//
	cbLightObject.ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f); //0.15f
	cbLightObject.diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cbLightObject.lightColor[0] = XMFLOAT4(0.964f, 0.415f, 0.211f, 1.0f);
	cbLightObject.lightColor[1] = XMFLOAT4(0.094f, 0.792f, 0.901f, 1.0f);
	cbLightObject.lightDirection = XMFLOAT3(0.0f, -0.5f, -0.2f);
	cbLightObject.specularPower = 64.0f;
	cbLightObject.lightPosition = XMFLOAT3(0.0f, 3.0f, 0.0f);
	cbLightObject.range = 100;

	//
	//Light constant buffer
	//
	memcpy(cbvGPUAddress[0] + ConstantCameraObjectAlignedSize, &cbLightObject, sizeof(cbLightObject));
	memcpy(cbvGPUAddress[1] + ConstantCameraObjectAlignedSize, &cbLightObject, sizeof(cbLightObject));
	memcpy(cbvGPUAddress[2] + ConstantCameraObjectAlignedSize, &cbLightObject, sizeof(cbLightObject));

	cbFireObject.frameTime = 0.0f;
	cbFireObject.scales = XMFLOAT3(1.0f, 2.0f, 3.0f);
	cbFireObject.scrollSpeed = XMFLOAT3(1.3f, 2.1f, 2.3f);
	XMStoreFloat4x4(&cbFireObject.worldMatrix, fireObject->transform.GetWorldMatrix());

	distortionObject.distortion1 = XMFLOAT2(0.1f, 0.2f);
	distortionObject.distortion2 = XMFLOAT2(0.1f, 0.3f);
	distortionObject.distortion3 = XMFLOAT2(0.1f, 0.1f);

	distortionObject.distortionBias = 0.5f;
	distortionObject.distortionScale = 0.8f;

	memcpy(cbvGPUAddress[0] + distortionLocation, &distortionObject, sizeof(distortionObject));
	memcpy(cbvGPUAddress[1] + distortionLocation, &distortionObject, sizeof(distortionObject));
	memcpy(cbvGPUAddress[2] + distortionLocation, &distortionObject, sizeof(distortionObject));

	return true;
}

void LittleEngineD12::Update(float deltaTime)
{
	if (LEState == State::Logo)
	{
		if (ALogo->HasFinished() || m_playerInput->IsLeftMouseButtonDown())
		{
			LEState = State::Menu;
			ALogo->transform.Position.x = 0.0f;
			ALogo->transform.Position.y = 1.0f;
		}

		//
		//LOGO
		//
		APlayer->Update(deltaTime);
		ALogo->Update(deltaTime);
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(ALogo->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[frameIndex] + ALogo->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));
		memcpy(cbvGPUAddress[frameIndex], &cbCameraObject, sizeof(cbCameraObject));
	}
	else if (LEState == State::Menu)
	{
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(gameLogo->transform.GetWorldMatrix()));
		memcpy(cbvGPUAddress[frameIndex] + gameLogo->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));

		if (m_playerInput->IsEnterKeyDown())
		{
			if (menuIndex == 0)
			{
				APlayer->SetToPerpsView(windowSize.x, windowSize.y);
				APlayer->CanPlayerMove(true);
				APlayer->ShowMesh(true);
				APlayer->Update(deltaTime);
				LEState = State::Playing;
			}
			else if (menuIndex == 1)
			{
				LEState = State::Stopping;
			}
		}

		//Move up the menu
		if (m_playerInput->KeyPress(VKey_W))
		{
			menuIndex++;
			if (menuIndex == indexMax)
			{
				menuIndex = 0;
			}
		}
		//move down the menu
		else if (m_playerInput->KeyPress(VKey_S))
		{
			menuIndex--;
			if (menuIndex < 0)
			{
				menuIndex = (indexMax - 1);
			}
		}


	}
	else if (LEState == State::Playing || LEState == State::Lost)
	{
		cbFireObject.frameTime += deltaTime;

		if (cbFireObject.frameTime > 1000.0f)
		{
			cbFireObject.frameTime = 0.0f;
		}

		//moves the uv
		cbCameraObject.uvMovement.x += (deltaTime / 10.0f);

		if (cbCameraObject.uvMovement.x >= 1)
			cbCameraObject.uvMovement.x -= 1;

		//Increase sine for shaders
		runningSin += (deltaTime / 4.0f);

		if (runningSin > maxRunning)
		{
			runningSin -= maxRunning;
		}
		cbCameraObject.uvMovement.y = sin(runningSin);

		//Updates the player/Camera position
		APlayer->Update(deltaTime);
		memcpy(cbvGPUAddress[frameIndex], &cbCameraObject, sizeof(cbCameraObject));

		//Update map main colour
		cbLightObject.lightColor[0] = ColourLerp(mainColor1, mainColor2, (cos(sin(runningSin)) * 2) - 1);
		memcpy(cbvGPUAddress[frameIndex] + ConstantCameraObjectAlignedSize, &cbLightObject, sizeof(cbLightObject));

		if (LETransform::Get2DDistanceBetweenTransforms(trailMiddle->transform, APlayer->transform) > trailDistance)
		{
			//Get position vector
			XMFLOAT3 newPosition;
			newPosition.x = APlayer->transform.Position.x - trailMiddle->transform.Position.x;
			newPosition.y = 0;
			newPosition.z = APlayer->transform.Position.z - trailMiddle->transform.Position.z;

			//Normalise
			XMFLOAT3 normal;
			XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&newPosition)));

			//Multiply by length
			normal = Multi(normal, trailDistance);

			//set new position
			trailMiddle->transform.Position.x += normal.x;
			trailMiddle->transform.Position.z += normal.z;
			trailMiddle->transform.Position.y = 0.0f;

			trailMatrix[currentTrail] = trailMiddle->transform.GetWorldMatrix();
			XMStoreFloat4x4(&trailInstances.worldMatrix[currentTrail++], XMMatrixTranspose(trailMiddle->transform.GetWorldMatrix()));

			if (createTrail) {
				trailMiddle->setNumberToRender(currentTrail);
				if (currentTrail == MAX_TRAILS)
				{
					createTrail = false;
				}
			}

			if (currentTrail == MAX_TRAILS)
			{
				currentTrail = 0;
			}
		}
		memcpy(cbvGPUAddress[frameIndex] + trailMiddle->GetConstantBufferOffset(), &trailInstances, sizeof(trailInstances));

		XMMATRIX worldMatrix;
		for (std::vector<std::shared_ptr<LEActor>>::iterator it = playerMesh.begin(); it != playerMesh.end(); it++)
		{
			(*it)->Update(deltaTime);
			worldMatrix = LETransform::GetOffsetMatrix((*it)->transform, APlayer->transform);

			//Store the new world matrix for the constant buffer and update the collision
			//Transpose the matrix for the graphic card
			XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(worldMatrix));
			memcpy(cbvGPUAddress[frameIndex] + (*it)->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));
		}

		//Player collider is the front wheel
		playerCollider->SetConstantRotation(XMFLOAT3(0.0f, 0.0f, APlayer->getCameraSpeed() / 10));
		playerCollider->UpdateCollision(LETransform::GetOffsetMatrix(playerCollider->transform, APlayer->transform));

		//
		//Fire Bike
		//
		XMStoreFloat4x4(&cbFireObject.worldMatrix, XMMatrixTranspose(LETransform::GetOffsetMatrix(fireObject->transform, APlayer->transform)));
		memcpy(cbvGPUAddress[frameIndex] + fireObject->GetConstantBufferOffset(), &cbFireObject, sizeof(cbFireObject));

		//
		//Speedo
		//
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(LETransform::GetOffsetMatrix(speedo->transform, APlayer->transform)));
		memcpy(cbvGPUAddress[frameIndex] + speedo->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));

		//
		//Speedo Needle
		//

		float lerpAlpha = MapFloat(APlayer->getCameraSpeed(), 0.0f, APlayer->getCameraMaxSpeed(), 0.0f, 1.0f);

		speedoNeedle->transform.FirstRotation.y = Lerp(needleMinAngle, needleMaxAngle, lerpAlpha);

		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(LETransform::GetOffsetMatrix(speedoNeedle->transform, APlayer->transform)));
		memcpy(cbvGPUAddress[frameIndex] + speedoNeedle->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));

		//
		//Player NEON
		//
		worldMatrix = LETransform::GetOffsetMatrix(playerNeon->transform, APlayer->transform);
		XMStoreFloat4x4(&cbPerObject.worldMatrix, XMMatrixTranspose(worldMatrix));
		memcpy(cbvGPUAddress[frameIndex] + playerNeon->GetConstantBufferOffset(), &cbPerObject, sizeof(cbPerObject));

		for (std::vector<std::shared_ptr<FireTexture>>::iterator it = fireList.begin(); it != fireList.end(); it++)
		{
			XMStoreFloat4x4(&cbFireObject.worldMatrix, XMMatrixTranspose((*it)->transform.GetWorldMatrix()));
			memcpy(cbvGPUAddress[frameIndex] + (*it)->GetConstantBufferOffset(), &cbFireObject, sizeof(cbFireObject));
		}

		if (LEState != State::Lost)
		{
			//
			//Collision
			//

			if (invunerableTimer <= 0)
			{
				//TODO:Collision
				for (std::vector<std::shared_ptr<LEActor>>::iterator it = collidableMapActorList.begin(); it != collidableMapActorList.end(); it++)
				{
					(*it)->UpdateCollision((*it)->transform.GetWorldMatrix());

					if (collision->AABBCollision((*it)->GetAABBCollider(), playerCollider->GetAABBCollider()))
					{
						PlayerLost();
					}
				}

				for (int i = 0; i < ((createTrail)? currentTrail : MAX_TRAILS); i++)
				{
					trailMiddle->UpdateCollision(trailMatrix[i]);

					if (collision->AABBCollision(trailMiddle->GetAABBCollider(), playerCollider->GetAABBCollider()))
					{
						PlayerLost();
					}
				}
			}
			else
			{
				invunerableTimer -= deltaTime;
			}

			//Time alive counter
			timeAlive += deltaTime;
		}
		else
		{
			if (m_playerInput->IsEnterKeyDown())
			{
				ResetGame();
			}
		}
		
	}
}

void LittleEngineD12::UpdatePipeline()
{
	HRESULT hr;

	//We have to wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	//We can only reset the allocator once the gpu is done with it
	//Ressetting an allcator frees the memory that the command list was stored in
	hr = commandAllocator[frameIndex]->Reset();

	if (FAILED(hr))
	{
		LEState = State::Stopping;
	}

	// reset the command list. by resetting the command list we are putting it into
	// a recording state so we can start recording commands into the command allocator.
	// the command allocator that we reference here may have multiple command lists
	// associated with it, but only one can be recording at any time. Make sure
	// that any other command lists associated to this command allocator are in
	// the closed state (not recording).
	// Here you will pass an initial pipeline state object as the second parameter,
	// but in this tutorial we are only clearing the rtv, and do not actually need
	// anything but an initial default pipeline, which is what we get by setting
	// the second parameter to NULL

	if (LEState == State::Logo || LEState == State::Menu)
	{
		hr = commandList->Reset(commandAllocator[frameIndex], shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Transparent_Texture_Shader));
	}
	else
	{
		hr = commandList->Reset(commandAllocator[frameIndex], shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Colour_Glow_Shader));
	}

	if (FAILED(hr))
	{
		LEState = State::Stopping;
	}

	//Record commands into the command list

	//transision the "frameIndex" render target from the present state to the render targert state so the command list draws to it starting from here
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//here we again get the handle to our current render target view so we can set it up st the render targer in the output merger state of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// get a handle to the depth/stencil buffer
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dSDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//set the render targer for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//Clear the render targer
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// clear the depth/stencil buffer
	commandList->ClearDepthStencilView(dSDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// set root signature
	commandList->SetGraphicsRootSignature(shaderManager->GetRootSignature()); // set the root signature

	// draw triangle
	commandList->RSSetViewports(1, &viewport); // set the viewports
	commandList->RSSetScissorRects(1, &scissorRect); // set the scissor rects
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // set the primitive topology

	// 0 is the camera matrix we always keep this the same
	// 1 is the light data
	commandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantCameraObjectAlignedSize);

	if (LEState == State::Logo)
	{
		//Logo use root descriptor index 3 for the texture
		ALogo->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
	}
	else if (LEState == State::Menu)
	{
		gameLogo->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
		textRenderer->RenderText(dSDescriptorHeap, shaderManager, std::wstring(L"Start"), 0, XMFLOAT2(0.45, 0.6), frameIndex, ((menuIndex == 0) ? XMFLOAT2(4.0f, 4.0f) : XMFLOAT2(2.0f,2.0f)), XMFLOAT2(0.0f, 0.0f), ((menuIndex == 0) ? mainColor1 : mainColor2));
		textRenderer->RenderText(dSDescriptorHeap, shaderManager, L"Stop", 5, XMFLOAT2(0.45, 0.7) , frameIndex, ((menuIndex == 1) ? XMFLOAT2(4.0f, 4.0f) : XMFLOAT2(2.0f, 2.0f)), XMFLOAT2(0.0f, 0.0f), ((menuIndex == 1) ? mainColor1 : mainColor2));
	}
	else if (LEState == State::Playing || LEState == State::Lost)
	{
		if (mapActorList.size() > 0)
		{
			for (std::vector<std::shared_ptr<LEActor>>::iterator it = mapActorList.begin(); it != mapActorList.end(); it++)
			{
				(*it)->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
			}
		}

		commandList->SetPipelineState(shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Building_Shader));

		if (collidableMapActorList.size() > 0)
		{
			for (std::vector<std::shared_ptr<LEActor>>::iterator it = collidableMapActorList.begin(); it != collidableMapActorList.end(); it++)
			{
				(*it)->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
			}
		}

		playerNeon->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		if (playerMesh.size() > 0)
		{
			commandList->SetPipelineState(shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Bump_Texture_Shader));

			for (std::vector<std::shared_ptr<LEActor>>::iterator it = playerMesh.begin(); it != playerMesh.end(); it++)
			{
				(*it)->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
			}
		}

		commandList->SetPipelineState(shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Speedo_Shader));
		speedo->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
		speedoNeedle->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		commandList->SetPipelineState(shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Fire_Shader));
		commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + distortionLocation);

		if (APlayer->GoingFast())
			fireObject->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		if (fireList.size() > 0)
		{
			for (std::vector<std::shared_ptr<FireTexture>>::iterator it = fireList.begin(); it != fireList.end(); it++)
			{
				(*it)->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());
			}
		}

		//Change shader for the trail
		commandList->SetPipelineState(shaderManager->GetPipelineState(ShaderManager::ShaderTypes::Trail_Shader));
		commandList->SetGraphicsRootConstantBufferView(1, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress() + ConstantCameraObjectAlignedSize);

		trailMiddle->Render(3, constantBufferUploadHeaps[frameIndex]->GetGPUVirtualAddress());

		if (LEState == State::Lost)
		{
			// draw the text
			textRenderer->RenderText(dSDescriptorHeap, shaderManager, std::wstring(L"You survived ") + std::to_wstring((int)trunc(timeAlive)) + std::wstring(L" seconds!"), 0, XMFLOAT2(0.36f, 0.3f), frameIndex, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
			textRenderer->RenderText(dSDescriptorHeap, shaderManager, std::wstring(L"Press Enter to restart!"), 29, XMFLOAT2(0.36f, 0.4f), frameIndex, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));
		}
		else
		{
			textRenderer->RenderText(dSDescriptorHeap, shaderManager, std::wstring(L"Time:") + std::to_wstring((int)timeAlive), 0, XMFLOAT2(0.02f, 0.01f), frameIndex, XMFLOAT2(2.0f, 2.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f));	
		}

	}

	//transition the "frameindex" render targert from the render target state to the present state. if the debug layer is enabled you will recieve warning
	//if the present is called on the render target when it is not he present state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		LEState = State::Stopping;
	}
}

void LittleEngineD12::Render()
{
	HRESULT hr;

	UpdatePipeline(); //update the pipeline by sending commands to the command queue

					  //Create and array of the command lists  (We are only using one)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	//Execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// this command goes in at the end of our command queue. we will know when our command queue 
	// has finished because the fence value will be set to "fenceValue" from the GPU since the command
	// queue is being executed on the GPU
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		LEState = State::Stopping;
	}

	// present the current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		LEState = State::Stopping;
	}
}

void LittleEngineD12::Cleanup()
{
	// get swapchain out of full screen before exiting
	BOOL fs = false;
	if (swapChain->GetFullscreenState(&fs, NULL))
		swapChain->SetFullscreenState(false, NULL);

	// wait for the gpu to finish all frames
	/*
	for (int i = 0; i < FRAMEBUFFERCOUNT; ++i)
	{
		frameIndex = i;
		UpdatePipeline();
		//WaitForPreviousFrame();
	}
	*/

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(rtvDescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < FRAMEBUFFERCOUNT; ++i)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};

	modelManager->CleanUp();

	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(dSDescriptorHeap);

	for (int i = 0; i < FRAMEBUFFERCOUNT; ++i)
	{
		constantBufferUploadHeaps[i]->Unmap(0, nullptr);
		SAFE_RELEASE(constantBufferUploadHeaps[i]);
	};

}

void LittleEngineD12::WaitForPreviousFrame()
{
	HRESULT hr;

	//swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	//if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	//the command queue since it has not reached the "commandQueue->Signal(fence, fencevalue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		//We have the fence create an event which is signaled once the fences current value is fencevalue
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			LEState = State::Stopping;
		}

		//We will wait until the fence has triggered the even that is currently value has reached "fenceValue". once it's value
		//has reached "fenceValue" we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//Increment fenceValue for the next frame
	fenceValue[frameIndex]++;

}
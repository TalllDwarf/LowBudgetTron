#pragma once
#include <cmath>
#include "Input.h"
#include "ShaderManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PCollision.h"
#include "TextRenderer.h"


//
//Actors
//
#include "PlayerActor.h"
#include "FireTexture.h"
#include "LALogo.h"
#include "TrailActor.h"

class LittleEngineD12
{
public:
	LittleEngineD12();
	~LittleEngineD12();

	Input* m_playerInput;

	float trailDistance = 11.189f;
	
	float runningSin;
	float maxRunning = (4 * XM_PI)/2;

	float needleMinAngle = -XM_PI/6.0f;
	float needleMaxAngle = ((5 * XM_PI) / 4);

	XMFLOAT4 mainColor1 = XMFLOAT4(0.964f, 0.415f, 0.211f, 1.0f);
	XMFLOAT4 mainColor2 = XMFLOAT4(0.094f, 0.792f, 0.901f, 1.0f);

	// direct3d stuff
	//const int frameBufferCount = 3; // number of buffers we want, 2 for double buffering, 3 for tripple buffering

	ID3D12Device* device; // direct3d device

	IDXGISwapChain3* swapChain; // swapchain used to switch between render targets

	ID3D12CommandQueue *commandQueue; // container for command lists

	ID3D12DescriptorHeap* rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

	ID3D12Resource* renderTargets[FRAMEBUFFERCOUNT]; // number of render targets equal to buffer count

	ID3D12CommandAllocator* commandAllocator[FRAMEBUFFERCOUNT]; // we want enough allocators for each buffer * number of threads (we only have one thread)

	ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame

	ID3D12Fence* fence[FRAMEBUFFERCOUNT];    // an object that is locked while our command list is being executed by the gpu. We need as many 
											 //as we have allocators (more if we want to know when the gpu is finished with an asset)

	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

	UINT64 fenceValue[FRAMEBUFFERCOUNT]; // this value is incremented each frame. each fence will have its own value	

	D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.

	D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

	ID3D12Resource *depthStencilBuffer; // This is the memory for our depth buffer, it will also be used for a stencil buffer in a later tutorial

	ID3D12DescriptorHeap *dSDescriptorHeap; //This is a heap for our depth/stencil buffer descriptor

	// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
	// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
	// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
	// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
	// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
	// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
	// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
	// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
	// being copied.
	int ConstantBufferObjectAlignedSize = (sizeof(ConstantBufferObject) + 255) & ~255;
	int ConstantCameraObjectAlignedSize = (sizeof(ConstantCameraBuffer) + 255) & ~255;
	int ConstantLightObjectAlignedSize = (sizeof(ConstantLightBuffer) + 255) & ~255;
	int ConstantSpotLightObjectAlignedSize = (sizeof(ConstantSpotLightBuffer) + 255) & ~255;
	int ConstantFireObjectAlignedSize = (sizeof(ConstantFireBufferObject) + 255) & ~255;
	int ConstantDistortionAlignedSize = (sizeof(DistortionBuffer) + 255) & ~255;
	int ConstantInstanceAllignedSize = (sizeof(InstanceBufferObject) + 255) & ~255;

	ConstantBufferObject cbPerObject; // this is the constant buffer data we will send to the gpu 
	ConstantCameraBuffer cbCameraObject;  // (which will be placed in the resource we created above)
	ConstantLightBuffer cbLightObject; //Holds out light data
	ConstantSpotLightBuffer cbSpotLightObject;
	
	ConstantFireBufferObject cbFireObject;
	DistortionBuffer distortionObject;
	UINT distortionLocation;


	//
	//Constant buffers
	//

	ID3D12Resource* constantBufferUploadHeaps[FRAMEBUFFERCOUNT]; // this is the memory on the gpu where constant buffers for each frame will be placed

	UINT8* cbvGPUAddress[FRAMEBUFFERCOUNT]; // this is a pointer to each of the constant buffer resource heaps

	//
	//Managers
	//
	std::shared_ptr<ShaderManager> shaderManager;
	std::shared_ptr<ModelManager> modelManager;
	std::shared_ptr<TextureManager> textureManager;
	std::shared_ptr<TextRenderer> textRenderer;

	std::shared_ptr<PCollision> collision;

	//Menu
	int menuIndex = 0;
	int indexMax = 2;


	//Special actors
	std::shared_ptr<PlayerActor> APlayer;
	std::shared_ptr<LEActor> playerCollider;
	std::shared_ptr<LEActor> playerNeon;
	std::vector<std::shared_ptr<LEActor>> playerMesh;

	//Speedo
	std::shared_ptr<LEActor> speedoNeedle;
	std::shared_ptr<LEActor> speedo;

	//
	//Trail
	//
	float timeAlive = 0.0f;
	float invunerableTimer = 2.0f;
	int currentTrail = 0;
	InstanceBufferObject trailInstances;
	XMMATRIX trailMatrix[MAX_TRAILS];
	std::shared_ptr<LEActor> spawnPoint;
	std::shared_ptr<TrailActor> trailMiddle;
	bool createTrail = true;

	int getTrailCount()
	{
		return currentTrail;
	}

	//
	//Fire
	//
	std::vector <std::shared_ptr<FireTexture>> fireList;
	std::shared_ptr<FireTexture> fireObject;

	std::shared_ptr<LALogo> ALogo;
	std::shared_ptr<LEActor> gameLogo;

	//Actor arrays	
	std::vector<std::shared_ptr<LEActor>> mapActorList;
	std::vector<std::shared_ptr<LEActor>> collidableMapActorList;

	XMFLOAT2 windowSize;

	void PlayerLost();

	void ResetGame();

	int frameIndex; // current rtv we are on

	int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)

						   // function declarations
	bool InitD3D(HWND hwnd, int width, int height, bool fullScreen, Input* playerInput); // initializes direct3d 12

	void Update(float deltaTime); // update the game logic

	void UpdatePipeline(); // update the direct3d pipeline (update command lists)

	void Render(); // execute the command list

	void Cleanup(); // release com ojects and clean up memory

	void WaitForPreviousFrame(); // wait until gpu is finished with command list

	bool IsRunning() { return (LEState != State::Stopping); }

private:

	enum State
	{
		Logo,
		Menu,
		Playing,
		Lost,
		Stopping
	};

	State LEState;

};
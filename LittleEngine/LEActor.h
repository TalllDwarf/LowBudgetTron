#pragma once

//Used for variadic function
#include <cstdarg>

#include "ModelImporter.h"
#include "TextureImporter.h"
#include "LETransform.h"
#include "CollisionAABB.h"
#include "CollisionSphere.h"

class LEActor
{
public:
	LEActor();
	LEActor(XMFLOAT4 startPosition);
	LEActor(XMFLOAT4 startPosition, XMFLOAT3 startRotation);
	LEActor(XMFLOAT4 startPosition, XMFLOAT3 startRotation, XMFLOAT3 startScale);
	~LEActor();

	LETransform transform;
	XMFLOAT3 Velocity;
	//Rotation of the object updated once per frame
	XMFLOAT3 UpdateRotation;

	std::shared_ptr<CollisionAABB> collisionAABB;
	std::shared_ptr<CollisionSphere> collisionSphere;

	//Uses variadic parameter allowing me to add different amnount of images to each actor
	bool Initialise(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::shared_ptr<ModelImporter> actorModel, int numOfImages... );

	//Creates AABB collision for the actor
	virtual void InitialiseAABBCollision();

	//Creates Sphere collision for the actor
	virtual void InitialiseSphereCollision();

	//Updates our collision with the new world matrix
	virtual void UpdateCollision(XMMATRIX& worldMatrix);

	//sets the constant buffer position where this actors data is stored
	void SetConstantBufferOffset(UINT constantBufferOffset) { ConstantBufferOffset = constantBufferOffset; };
	UINT GetConstantBufferOffset() { return ConstantBufferOffset; }

	//Update function needs to be implemented by all actors
	virtual void Update(float deltaTime);

	//Render the model to the screen
	virtual void Render(UINT textureRootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAdress);

	//Resets the Actor to its starting position
	virtual void Reset() {};

	//set the update rotation, this rotates the 
	void SetConstantRotation(XMFLOAT3 constRotation) { UpdateRotation = constRotation; updateRotate = true; }

	//Return our colliders
	std::shared_ptr<CollisionAABB> GetAABBCollider() { return collisionAABB; }
	std::shared_ptr<CollisionSphere> GetSphereCollider() { return collisionSphere; }

	//Does our actor have a collider
	bool hasAABB() { return (collisionAABB != nullptr); }
	bool hasSphereCollision() { return (collisionSphere != nullptr); }

protected:

	int numberOfImages;

	UINT descriptorSize;
	UINT ConstantBufferOffset;

	ID3D12Device * m_device;
	ID3D12GraphicsCommandList* m_commandList;

	std::shared_ptr<ModelImporter> model;
	//Descriptor heap - contains texture
	ID3D12DescriptorHeap* m_textureDescriptorHeap;

	int rtvIncrementSize;

private:

	bool updateRotate;
	bool updateMove;
};
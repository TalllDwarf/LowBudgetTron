#include "CollisionAABB.h"



CollisionAABB::CollisionAABB()
{
}


CollisionAABB::~CollisionAABB()
{
}

//Gets the local AABB
void CollisionAABB::CreateAABB(std::shared_ptr<ModelImporter> model)
{
	std::vector<Vertex>* modelVertex = model->GetModelVectex();

	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (std::vector<Vertex>::iterator it = modelVertex->begin(); it != modelVertex->end(); it++)
	{
		//get the min and max x y z of each vertex this may not be a single vertex
		//but points on many vertex
		minVertex.x = min(minVertex.x, (*it).pos.x);
		minVertex.y = min(minVertex.y, (*it).pos.y);
		minVertex.z = min(minVertex.z, (*it).pos.z);

		maxVertex.x = max(maxVertex.x, (*it).pos.x);
		maxVertex.y = max(maxVertex.y, (*it).pos.y);
		maxVertex.z = max(maxVertex.z, (*it).pos.z);
	}

	//minLocalBoundingBox = XMFLOAT3(minVertex.x, minVertex.y, minVertex.z);
	//maxLocalBoundingBox = XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z);

	//Front face
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, minVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z));

	//Back face
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z));
}

void CollisionAABB::CreateAABB(std::vector<Vertex> modelVertex)
{
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (std::vector<Vertex>::iterator it = modelVertex.begin(); it != modelVertex.end(); it++)
	{
		//get the min and max x y z of each vertex this may not be a single vertex
		//but points on many vertex
		minVertex.x = min(minVertex.x, (*it).pos.x);
		minVertex.y = min(minVertex.y, (*it).pos.y);
		minVertex.z = min(minVertex.z, (*it).pos.z);

		maxVertex.x = max(maxVertex.x, (*it).pos.x);
		maxVertex.y = max(maxVertex.y, (*it).pos.y);
		maxVertex.z = max(maxVertex.z, (*it).pos.z);
	}

	//minLocalBoundingBox = XMFLOAT3(minVertex.x, minVertex.y, minVertex.z);
	//maxLocalBoundingBox = XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z);

	//Front face
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, minVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, maxVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, minVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, minVertex.y, minVertex.z));

	//Back face
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, minVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, minVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z));
	localBoundingBox.push_back(XMFLOAT3(minVertex.x, maxVertex.y, maxVertex.z));
}

void CollisionAABB::CreateAABB(XMFLOAT4 position, float size)
{
}

//Calculates the local AABB in the world 
void CollisionAABB::CalculateWorldAABB(XMMATRIX worldSpace)
{
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int i = 0; i < 8; i++)
	{
		XMVECTOR vert = XMVectorSet(localBoundingBox[i].x, localBoundingBox[i].y, localBoundingBox[i].z, 1.0f);
		vert = XMVector3TransformCoord(vert, worldSpace);

		minVertex.x = min(minVertex.x, XMVectorGetX(vert));
		minVertex.y = min(minVertex.y, XMVectorGetY(vert));
		minVertex.z = min(minVertex.z, XMVectorGetZ(vert));

		maxVertex.x = max(maxVertex.x, XMVectorGetX(vert));
		maxVertex.y = max(maxVertex.y, XMVectorGetY(vert));
		maxVertex.z = max(maxVertex.z, XMVectorGetZ(vert));
	}

	//Set World bounding box 
	minWorldBoundingBox = XMFLOAT3(minVertex.x, minVertex.y, minVertex.z);
	maxWorldBoundingBox = XMFLOAT3(maxVertex.x, maxVertex.y, maxVertex.z);
}
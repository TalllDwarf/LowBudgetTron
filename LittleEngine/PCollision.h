#pragma once
#include "CollisionAABB.h"
#include "CollisionSphere.h"

class PCollision
{
public:
	PCollision();
	~PCollision();

	//Checks if two AABB collide
	bool AABBCollision(std::shared_ptr<CollisionAABB> collBoxA, std::shared_ptr<CollisionAABB> collBoxB);

	//Checks if a AABB collides with a Sphere
	bool AABBSphereCollision(std::shared_ptr<CollisionAABB> collBox, std::shared_ptr<CollisionSphere> collSphere, XMFLOAT3 spherePosition);

	//Checks if two SpheresCollide
	bool SphereCollision(std::shared_ptr<CollisionSphere> collSphereA, XMFLOAT3 positionA, std::shared_ptr<CollisionSphere> collSphereB, XMFLOAT3 positionB);

private:

	//returns the squared distance between a sphere point and AABB collider
	float SquaredDistPointAABB(XMFLOAT3 spherePoint, std::shared_ptr<CollisionAABB> aabb);
};


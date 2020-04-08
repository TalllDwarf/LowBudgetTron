#include "PCollision.h"



PCollision::PCollision()
{
}


PCollision::~PCollision()
{
}

XMFLOAT3 operator+(const XMFLOAT3& a, const XMFLOAT3& b)
{
	return XMFLOAT3(a.x + b.y, a.y + b.y, a.z + b.z);
}

XMFLOAT3 operator-(const XMFLOAT3& a, const XMFLOAT3& b)
{
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

//Checks if two AABB collide
bool PCollision::AABBCollision(std::shared_ptr<CollisionAABB> collBoxA, std::shared_ptr<CollisionAABB> collBoxB)
{
	bool x = (collBoxA->maxWorldBoundingBox.x > collBoxB->minWorldBoundingBox.x) && (collBoxA->minWorldBoundingBox.x < collBoxB->maxWorldBoundingBox.x);
	bool y = (collBoxA->maxWorldBoundingBox.y > collBoxB->minWorldBoundingBox.y) && (collBoxA->minWorldBoundingBox.y < collBoxB->maxWorldBoundingBox.y);
	bool z = (collBoxA->maxWorldBoundingBox.z > collBoxB->minWorldBoundingBox.z) && (collBoxA->minWorldBoundingBox.z < collBoxB->maxWorldBoundingBox.z);

	return x && (y && z);
}

//Checks if AABB collides with a sphere
bool PCollision::AABBSphereCollision(std::shared_ptr<CollisionAABB> collBox, std::shared_ptr<CollisionSphere> collSphere, XMFLOAT3 spherePosition)
{
	float squaredDist = SquaredDistPointAABB((collSphere->centerOffset + spherePosition), collBox);

	return squaredDist <= (collSphere->radius * collSphere->radius);
}

//Returns the squared distance of a point and AABB collider
float PCollision::SquaredDistPointAABB(XMFLOAT3 spherePoint, std::shared_ptr<CollisionAABB> aabb)
{
	auto getPoint = 
	[&](float point, float axisMin, float axisMax) -> float
	{
		float out = 0;
		
		if (point < axisMin)
		{
			float val = (axisMin - point);
			out += val * val;
		}

		if (point > axisMax)
		{
			float val = (point - axisMax);
			out += val * val;
		}

		return out;
	};

	float squaredDist = 0.0f;

	squaredDist += getPoint(spherePoint.x, aabb->minWorldBoundingBox.x, aabb->maxWorldBoundingBox.x);
	squaredDist += getPoint(spherePoint.y, aabb->minWorldBoundingBox.y, aabb->maxWorldBoundingBox.y);
	squaredDist += getPoint(spherePoint.z, aabb->minWorldBoundingBox.z, aabb->maxWorldBoundingBox.z);

	return squaredDist;
}

//return true if two spheres collide
bool PCollision::SphereCollision(std::shared_ptr<CollisionSphere> collSphereA, XMFLOAT3 positionA, std::shared_ptr<CollisionSphere> collSphereB, XMFLOAT3 positionB)
{
	//Calculate the difference 
	XMFLOAT3 differenctVector = (collSphereA->centerOffset + positionA) - (collSphereB->centerOffset + positionB);

	//Calculate the length of the vector
	float length = sqrt((differenctVector.x * differenctVector.x) + (differenctVector.y * differenctVector.y) + (differenctVector.z * differenctVector.z));

	//Get the sum of the radiuses
	float radiusSum = collSphereA->radius + collSphereB->radius;

	//if length is less than the sum of the radiuses 
	//We have collision
	return (length < radiusSum);
}
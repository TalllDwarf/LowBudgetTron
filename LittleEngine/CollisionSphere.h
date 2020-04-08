#pragma once
#include "ModelImporter.h"

class CollisionSphere
{
public:
	CollisionSphere();
	~CollisionSphere();

	XMFLOAT3 centerOffset;
	float radius;

	void CreateSphereCollider(std::shared_ptr<ModelImporter> model);

	//TODO:Update Sphere
};


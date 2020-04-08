#include "CollisionSphere.h"

CollisionSphere::CollisionSphere()
{
}


CollisionSphere::~CollisionSphere()
{
}

void CollisionSphere::CreateSphereCollider(std::shared_ptr<ModelImporter> model)
{
	std::vector<Vertex>* modelVertex = model->GetModelVectex();

	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (std::vector<Vertex>::iterator it = modelVertex->begin(); it != modelVertex->end(); it++)
	{
		// The minVertex and maxVertex will most likely not be actual vertices in the model, but vertices
		// that use the smallest and largest x, y, and z values from the model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, (*it).pos.x);    // Find smallest x value in model
		minVertex.y = min(minVertex.y, (*it).pos.y);    // Find smallest y value in model
		minVertex.z = min(minVertex.z, (*it).pos.z);    // Find smallest z value in model

															  //Get the largest vertex 
		maxVertex.x = max(maxVertex.x, (*it).pos.x);    // Find largest x value in model
		maxVertex.y = max(maxVertex.y, (*it).pos.y);    // Find largest y value in model
		maxVertex.z = max(maxVertex.z, (*it).pos.z);    // Find largest z value in model
	}

	// Compute distance between maxVertex and minVertex
	float distX = (maxVertex.x - minVertex.x) / 2.0f;
	float distY = (maxVertex.y - minVertex.y) / 2.0f;
	float distZ = (maxVertex.z - minVertex.z) / 2.0f;

	// Now store the distance between (0, 0, 0) in model space to the models real center
	centerOffset = XMFLOAT3(maxVertex.x - distX, maxVertex.y - distY, maxVertex.z - distZ);

	// Compute bounding sphere (distance between min and max bounding box vertices)
	// boundingSphere = sqrt(distX*distX + distY*distY + distZ*distZ) / 2.0f;
	radius = XMVectorGetX(XMVector3Length(XMVectorSet(distX, distY, distZ, 0.0f)));
}

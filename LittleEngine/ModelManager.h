#pragma once
#include <map>
#include "stdafx.h"
#include "ModelImporter.h"

namespace Model
{
	static const char* PLANE = "Model/Plane.obj";
	static const char* CUBE = "Model/Cube.obj";

	static const char* FLOOR = "Model/Building/Floor.obj";
	static const char* WALL = "Model/Building/Wall.obj";

	static const char* ROOF = "Model/Building/Roof.obj";
	static const char* SIDES = "Model/Building/Sides.obj";

	static const char* BIKE = "Model/Riding/Tron-Body.obj";
	static const char* BIKE_NEON = "Model/Riding/Tron-Neon.obj";
	static const char* BIKE_WHEEL = "Model/Riding/Tron-Wheel-RIM.obj";

	static const char* BIKE_SPEEDO_NEEDLE = "Model/Riding/SpeedoNeedle.obj";
	static const char* BIKE_SPEEDO = "Model/Riding/Speedo.obj";

	static const char* TRAIL_START = "Model/Riding/TrailStart.obj";
	static const char* TRAIL_MIDDLE = "Model/Riding/Trail.obj";

	static const char* FIRE_WALL = "Model/Fire.obj";
};

class ModelManager
{

public:
	ModelManager();
	~ModelManager();

	//Set device and commandlist variables
	//Then loads all available models
	bool Initialise(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	//Return a loaded model
	std::shared_ptr<ModelImporter> GetModel(const char* modelPath);	

	void CleanUp();

private:

	std::shared_ptr<ModelImporter> LoadModel(const char* model);

	ID3D12Device * m_device;
	ID3D12GraphicsCommandList* m_commandList;

	//All of our loaded models
	std::map<const char*, std::shared_ptr<ModelImporter>> models;	

};


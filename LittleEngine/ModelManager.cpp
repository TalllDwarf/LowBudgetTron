#include "ModelManager.h"



ModelManager::ModelManager()
{
}


ModelManager::~ModelManager()
{
}

bool ModelManager::Initialise(ID3D12Device * device, ID3D12GraphicsCommandList * commandList)
{
	m_device = device;
	m_commandList = commandList;
	
	return true;
}

//Get a model that uses a specific path
std::shared_ptr<ModelImporter> ModelManager::GetModel(const char* modelPath)
{
	std::map<const char *, std::shared_ptr<ModelImporter>>::iterator it;

	it = models.find(modelPath);

	if (it == models.end())
	{
		return LoadModel(modelPath);		
	}
	
	return (*it).second;
}

//Tells each model importer to clean up
void ModelManager::CleanUp()
{
	for (std::map<const char*, std::shared_ptr<ModelImporter>>::iterator it = models.begin(); it != models.end(); it++)
	{
		it->second->CleanUp();
	}
}

//Create a model loader and tell it what model we want it to load
std::shared_ptr<ModelImporter> ModelManager::LoadModel(const char* model)
{
	std::shared_ptr<ModelImporter> temp = std::make_shared<ModelImporter>();
	temp->Init(m_device, m_commandList);

	if (!temp->OpenFile(model))
	{
		return nullptr;
	}

	models[model] = temp;
	return temp;
}

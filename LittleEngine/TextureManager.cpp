#include "TextureManager.h"

TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
}

//Loads all the textures we need
bool TextureManager::Initialise(ID3D12Device * device, ID3D12GraphicsCommandList * commandList)
{
	m_device = device;
	m_commandList = commandList;

	return true;
}

//Recives a texture from the textures map
std::shared_ptr<TextureImporter> TextureManager::GetTexture(const wchar_t * texturePath)
{
	std::map<const wchar_t*, std::shared_ptr<TextureImporter>>::iterator it;

	it = textures.find(texturePath);

	//If not found
	if (it == textures.end())
	{
		return LoadTexture(texturePath);
	}
		
	//return the texture we found
	return (*it).second;
}

//Cleans up all the textures that we loaded
void TextureManager::TextureCleanup()
{
	for (std::map<const wchar_t*, std::shared_ptr<TextureImporter>>::iterator it = textures.begin(); it != textures.end(); it++)
	{
		it->second->TextureCleanup();
	}
}

//Load a texture then save it in the textures map
std::shared_ptr<TextureImporter> TextureManager::LoadTexture(const wchar_t* texturePath)
{
	std::shared_ptr<TextureImporter> temp = std::make_shared<TextureImporter>();
	temp->Initialise(m_device, m_commandList);
	
	if (!temp->LoadTexture(texturePath))
	{
		return nullptr;
	}
	
	textures[texturePath] = temp;

	return temp;
}

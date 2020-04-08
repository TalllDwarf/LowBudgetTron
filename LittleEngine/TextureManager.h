#pragma once
#include <map>
#include "TextureImporter.h"

namespace Texture
{
	static const wchar_t* LOGO = L"Texture/LittleEngine.png";
	static const wchar_t* TRON = L"Texture/TronBudget.jpg";

	static const wchar_t* METAL_5_COLOR = L"Texture/Texture/Metal_Panel_005_COLOR.jpg";
	static const wchar_t* METAL_5_NORMAL = L"Texture/Normal/Metal_Panel_005_NORM.jpg";
	static const wchar_t* METAL_5_SPEC = L"Texture/Spec/Metal_Panel_005_OCC.jpg";
	static const wchar_t* METAL_5_LIGHT = L"Texture/Light/Metal_Panel_005_LIGHT.jpg";

	static const wchar_t* TRAIL_1_COLOR = L"Texture/Texture/Trail_Color.png";
	static const wchar_t* TRAIL_2_COLOR = L"Texture/Texture/Trail_Mask.png";
	static const wchar_t* TRAIL_1_LIGHT = L"Texture/Light/Abstract_001_LIGHT.jpg";	

	static const wchar_t* FLOOR_1_COLOR = L"Texture/Texture/Metal_galvanized_001_COLOR.jpg";
	static const wchar_t* FLOOR_1_NORMAL = L"Texture/Normal/Metal_galvanized_001_NRM.jpg";
	static const wchar_t* FLOOR_1_SPEC = L"Texture/Spec/Metal_galvanized_001_SPEC.jpg";
	static const wchar_t* FLOOR_1_LIGHT = L"Texture/Light/Edge_Light.jpg";

	static const wchar_t* BIKE_BASE = L"Texture/Texture/DarkMetal.jpg";
	static const wchar_t* BIKE_TIRE = L"Texture/Texture/Tire.jpg";

	static const wchar_t* BIKE_SPEEDO = L"Texture/Texture/SpeedoTexture.png";

	static const wchar_t* FIRE = L"Texture/Fire/flame.gif";
	static const wchar_t* FIRE_NOISE = L"Texture/Fire/noise.gif";
	static const wchar_t* FIRE_ALPHA = L"Texture/Fire/transparent.gif";
	static const wchar_t* FIRE_BIG_ALPHA = L"Texture/Fire/bigTransparent.gif";
}

class TextureManager
{
public:
	TextureManager();
	~TextureManager();

	bool Initialise(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	std::shared_ptr<TextureImporter> GetTexture(const wchar_t* texturePath);

	void TextureCleanup();

private:

	ID3D12Device * m_device;
	ID3D12GraphicsCommandList* m_commandList;

	std::map<const wchar_t*, std::shared_ptr<TextureImporter>> textures;

	std::shared_ptr<TextureImporter> LoadTexture(const wchar_t* texturePath);

};


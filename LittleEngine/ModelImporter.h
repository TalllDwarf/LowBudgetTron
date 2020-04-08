#pragma once
#include <vector>
#include "stdafx.h"
#include "d3dx12.h"

class ModelImporter
{
public:
	ModelImporter();
	~ModelImporter();

	void Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	bool OpenFile(const char* objPath);

	void CalculateBinormalTangent();
	void CalculateBT(TempVertex v1, TempVertex v2, TempVertex v3, XMFLOAT3& tangent, XMFLOAT3& binormal);

	void CalcualteNewNormal(XMFLOAT3 tangent, XMFLOAT3 binormal, XMFLOAT3& normal);

	void SetBuffers();

	void CommitResource();
	void CleanUp();

	BYTE* GetVList();
	BYTE* GetIList();	

	//Returns the vector with the models vertex
	std::vector<Vertex>* GetModelVectex() { return &vVector; }
		
	int GetIndeciesCount();

private:

	int vBufferSize;
	int iBufferSize;
	int numOfIndecies;

	BYTE* vList;
	BYTE* iList;
	std::vector<Vertex> vVector;
	std::vector<DWORD> iVector;

	ID3D12Resource *m_pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ID3D12Resource *m_pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	ID3D12Device * m_pDevice;
	ID3D12GraphicsCommandList* m_pCommandList;


};


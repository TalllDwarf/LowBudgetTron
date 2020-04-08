#pragma once

#include "stdafx.h"

class Shader
{
public:

	virtual bool Initialise(ID3D12Device* device, ID3D12RootSignature* rootSignature, DXGI_SAMPLE_DESC sampleDesc) = 0;

	ID3D12PipelineState* GetPipelineState() { return pipelineStateObject; }

	void Cleanup() { SAFE_RELEASE(pipelineStateObject); }

protected:

	ID3D12PipelineState * pipelineStateObject; // pso containing a pipeline state

};
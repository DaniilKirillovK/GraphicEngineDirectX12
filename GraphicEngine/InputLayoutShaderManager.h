#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"


class InputLayoutShaderManager
{
public:
	static void BuildShadersAndInputLayout(std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>>& inputLayouts,
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>& shaders);
};


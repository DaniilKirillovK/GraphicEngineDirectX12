#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"


class InputLayoutShaderManager
{
public:
	static std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;
	static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;


	static void BuildShadersAndInputLayout();
};


#pragma once

#include "SamplerManager.h"

static struct RootSignatureManager
{
public:
    static void BuildRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device,
        std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>>& rootSignatures);

private:
    static void CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc, std::string rootSigName, Microsoft::WRL::ComPtr<ID3D12Device> device,
        std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>>& rootSignatures);
};


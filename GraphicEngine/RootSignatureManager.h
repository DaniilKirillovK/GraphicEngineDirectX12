#pragma once

#include "SamplerManager.h"

static struct RootSignatureManager
{
public:
    static std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures;

    static void BuildRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device);

private:
    static void CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc, std::string rootSigName, Microsoft::WRL::ComPtr<ID3D12Device> device,
        std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>>& rootSignatures);
};


#pragma once
#include <d3d12.h>
#include "d3dUtil.h"

struct ExtractBrightConstants 
{
    float threshold;
    float softThreshold;
    DirectX::XMFLOAT2 Pad;
};

struct BlurConstants
{
    DirectX::XMFLOAT4 weights1;
    DirectX::XMFLOAT4 weights2;
    DirectX::XMFLOAT4 weights3;
    DirectX::XMFLOAT4 weights4;
    DirectX::XMFLOAT4 weights5;
    DirectX::XMFLOAT4 weights6;
    DirectX::XMFLOAT4 weights7;
    DirectX::XMFLOAT4 weights8;
    DirectX::XMFLOAT4 weights9;
    DirectX::XMFLOAT4 weights10;

    DirectX::XMFLOAT4 offsets1;
    DirectX::XMFLOAT4 offsets2;
    DirectX::XMFLOAT4 offsets3;
    DirectX::XMFLOAT4 offsets4;
    DirectX::XMFLOAT4 offsets5;
    DirectX::XMFLOAT4 offsets6;
    DirectX::XMFLOAT4 offsets7;
    DirectX::XMFLOAT4 offsets8;
    DirectX::XMFLOAT4 offsets9;
    DirectX::XMFLOAT4 offsets10;

    int blurRadius;
    int horizontal;
    float texOffset;
    float pad3;
};

struct CombineConstants
{
    float bloomInstensity;
    DirectX::XMFLOAT3 Pad;
};

inline void FillBlurConstants(BlurConstants& constants, int radius, float width, float height, bool horizontal) 
{
    float sigma = radius / 2.0f;

    float weightSum = 0.0f;
    std::vector<float> weights(40);
    std::vector<float> offsets(40);

    for (int i = -radius; i <= radius; ++i) 
    {
        float x = (float)i;
        weights[i + radius] = expf(-x * x / (2.0f * sigma * sigma));
        weightSum += weights[i + radius];
    }

    for (int i = 0; i < 2 * radius + 1; ++i) 
    {
        weights[i] /= weightSum;
    }

    float texOffset = 0.0f;
    if (horizontal)
    {
        texOffset = 1 / width;
    }
    else
    {
        texOffset = 1 / height;
    }

    for (int i = -radius; i <= radius; ++i) 
    {
        if (horizontal) 
        {
            offsets[i + radius] = i * 1 / width;
        }
        else 
        {
            offsets[i + radius] = i * 1 / height;
        }
    }

    constants.blurRadius = radius;
    constants.texOffset = texOffset;
    constants.horizontal = (int)horizontal;

    constants.weights1 = DirectX::XMFLOAT4(weights[0], weights[1], weights[2], weights[3]);
    constants.weights2 = DirectX::XMFLOAT4(weights[4], weights[5], weights[6], weights[7]);
    constants.weights3 = DirectX::XMFLOAT4(weights[8], weights[9], weights[10], weights[11]);
    constants.weights4 = DirectX::XMFLOAT4(weights[12], weights[13], weights[14], weights[15]);
    constants.weights5 = DirectX::XMFLOAT4(weights[16], weights[17], weights[18], weights[19]);
    constants.weights6 = DirectX::XMFLOAT4(weights[20], weights[21], weights[22], weights[23]);
    constants.weights7 = DirectX::XMFLOAT4(weights[24], weights[25], weights[26], weights[27]);
    constants.weights8 = DirectX::XMFLOAT4(weights[28], weights[29], weights[30], weights[31]);
    constants.weights9 = DirectX::XMFLOAT4(weights[32], weights[33], weights[34], weights[35]);
    constants.weights10 = DirectX::XMFLOAT4(weights[36], weights[37], weights[38], weights[39]);

    constants.offsets1 = DirectX::XMFLOAT4(offsets[0], offsets[1], offsets[2], offsets[3]);
    constants.offsets2 = DirectX::XMFLOAT4(offsets[4], offsets[5], offsets[6], offsets[7]);
    constants.offsets3 = DirectX::XMFLOAT4(offsets[8], offsets[9], offsets[10], offsets[11]);
    constants.offsets4 = DirectX::XMFLOAT4(offsets[12], offsets[13], offsets[14], offsets[15]);
    constants.offsets5 = DirectX::XMFLOAT4(offsets[16], offsets[17], offsets[18], offsets[19]);
    constants.offsets6 = DirectX::XMFLOAT4(offsets[20], offsets[21], offsets[22], offsets[23]);
    constants.offsets7 = DirectX::XMFLOAT4(offsets[24], offsets[25], offsets[26], offsets[27]);
    constants.offsets8 = DirectX::XMFLOAT4(offsets[28], offsets[29], offsets[30], offsets[31]);
    constants.offsets9 = DirectX::XMFLOAT4(offsets[32], offsets[33], offsets[34], offsets[35]);
    constants.offsets10 = DirectX::XMFLOAT4(offsets[36], offsets[37], offsets[38], offsets[39]);
}

class Bloom
{
public:
    static void Init(Microsoft::WRL::ComPtr<ID3D12Device> device, UINT width, UINT height);

    static Microsoft::WRL::ComPtr<ID3D12Resource> mSourceTexture;
    static Microsoft::WRL::ComPtr<ID3D12Resource> mBrightnessTexture;
    static Microsoft::WRL::ComPtr<ID3D12Resource> mBlurTextures[2];
    static Microsoft::WRL::ComPtr<ID3D12Resource> mBloomResultTexture;
    static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;

    static UINT mWidth, mHeight;
};

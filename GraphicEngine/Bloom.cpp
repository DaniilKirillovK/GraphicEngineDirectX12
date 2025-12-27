#include "Bloom.h"

Microsoft::WRL::ComPtr<ID3D12Resource> Bloom::mSourceTexture;
Microsoft::WRL::ComPtr<ID3D12Resource> Bloom::mBrightnessTexture;
Microsoft::WRL::ComPtr<ID3D12Resource> Bloom::mBlurTextures[2];
Microsoft::WRL::ComPtr<ID3D12Resource> Bloom::mBloomResultTexture;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Bloom::mRtvHeap;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> Bloom::mSrvHeap;
UINT Bloom::mWidth, Bloom::mHeight;

void Bloom::Init(Microsoft::WRL::ComPtr<ID3D12Device> device, UINT width, UINT height)
{
	mWidth = width;
	mHeight = height;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = width / 2;
    texDesc.Height = height / 2;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f; 
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue,
        IID_PPV_ARGS(&mBrightnessTexture));

    for (int i = 0; i < 2; ++i)
    {
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearValue,
            IID_PPV_ARGS(&mBlurTextures[i]));
    }

    D3D12_RESOURCE_DESC tex2Desc = {};
    tex2Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tex2Desc.Alignment = 0;
    tex2Desc.Width = width;
    tex2Desc.Height = height;
    tex2Desc.DepthOrArraySize = 1;
    tex2Desc.MipLevels = 1;
    tex2Desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    tex2Desc.SampleDesc.Count = 1;
    tex2Desc.SampleDesc.Quality = 0;
    tex2Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    tex2Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &tex2Desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue,
        IID_PPV_ARGS(&mSourceTexture));

    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &tex2Desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue,
        IID_PPV_ARGS(&mBloomResultTexture));

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 5;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

    UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < 5; ++i) 
    {
        ID3D12Resource* texture = nullptr;
        switch (i) 
        {
        case 0: 
            texture = mBrightnessTexture.Get(); 
            break;
        case 1: 
            texture = mBlurTextures[0].Get(); 
            break;
        case 2: 
            texture = mBlurTextures[1].Get(); 
            break;
        case 3: 
            texture = mBloomResultTexture.Get();
            break;
        case 4:
            texture = mSourceTexture.Get();
            break;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        device->CreateRenderTargetView(texture, &rtvDesc, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 5;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap));

    UINT srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(mSrvHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < 5; ++i) 
    {
        ID3D12Resource* texture = nullptr;
        switch (i)
        {
        case 0:
            texture = mBrightnessTexture.Get();
            break;
        case 1:
            texture = mBlurTextures[0].Get();
            break;
        case 2:
            texture = mBlurTextures[1].Get();
            break;
        case 3:
            texture = mBloomResultTexture.Get();
            break;
        case 4:
            texture = mSourceTexture.Get();
            break;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        device->CreateShaderResourceView(texture, &srvDesc, srvHandle);
        srvHandle.Offset(1, srvDescriptorSize);
    }
}

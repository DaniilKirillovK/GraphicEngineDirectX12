#pragma once
#include "MeshPipelineModel.h"
#include "DescriptorHeapManager.h"

#define MAX_VERTS 64
#define MAX_PRIMS 126
#define MAX_LOD_LEVELS 8

enum DescriptorHeapIndex
{
    SRV_MeshInfoLODs = 0,
    SRV_VertexLODs = SRV_MeshInfoLODs + MAX_LOD_LEVELS,
    SRV_MeshletLODs = SRV_VertexLODs + MAX_LOD_LEVELS,
    SRV_UniqueVertexIndexLODs = SRV_MeshletLODs + MAX_LOD_LEVELS,
    SRV_PrimitiveIndexLODs = SRV_UniqueVertexIndexLODs + MAX_LOD_LEVELS,
    SRV_Count = SRV_PrimitiveIndexLODs + MAX_LOD_LEVELS,
};

enum class RenderMode
{
    Flat,
    Meshlets,
    LOD,
    Count
};

template <typename T>
T GetAlignedSize(T size)
{
    const T alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    const T alignedSize = (size + alignment - 1) & ~(alignment - 1);
    return alignedSize;
}

template <typename T, typename U>
T DivRoundUp(T num, U denom)
{
    return (num + denom - 1) / denom;
}

_declspec(align(256u)) struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4X4 WorldView;
    DirectX::XMFLOAT4X4 WorldViewProj;
    uint32_t   DrawMeshlets;
};

struct MeshPipelineShader
{
    byte* data;
    uint32_t size;
};

static class MeshPipeline
{
public:

    static void LoadAssets(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

    static UINT8* m_cbvDataBegin;
    static SceneConstantBuffer m_constantBufferData;
    static MeshPipelineModel m_model;
    static Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    static const wchar_t* c_meshFilename;
    static const wchar_t* c_meshShaderFilename;
    static const wchar_t* c_pixelShaderFilename;

    static MeshPipelineShader meshShader;
    static MeshPipelineShader pixelShader;
};


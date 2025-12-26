#include "MeshPipeline.h"

const wchar_t* MeshPipeline::c_meshFilename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Models\\dropship.bin";
MeshPipelineModel MeshPipeline::m_model = MeshPipelineModel();
Microsoft::WRL::ComPtr<ID3D12Resource> MeshPipeline::m_constantBuffer = Microsoft::WRL::ComPtr<ID3D12Resource>();
UINT8* MeshPipeline::m_cbvDataBegin = nullptr;
SceneConstantBuffer MeshPipeline::m_constantBufferData = SceneConstantBuffer();
MeshPipelineShader MeshPipeline::meshShader = MeshPipelineShader();
MeshPipelineShader MeshPipeline::pixelShader = MeshPipelineShader();

const wchar_t* MeshPipeline::c_meshShaderFilename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\x64\\Debug\\MeshletMS.cso";
const wchar_t* MeshPipeline::c_pixelShaderFilename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\x64\\Debug\\MeshletPS.cso";

void MeshPipeline::LoadAssets(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    m_model.LoadFromFile(c_meshFilename);
    m_model.UploadGpuResources(device.Get(), commandQueue.Get(), commandAllocator.Get(), commandList.Get());

    ReadDataFromFile(c_meshShaderFilename, &meshShader.data, &meshShader.size);
    ReadDataFromFile(c_pixelShaderFilename, &pixelShader.data, &pixelShader.size);

    const UINT64 constantBufferSize = sizeof(SceneConstantBuffer);

    const CD3DX12_HEAP_PROPERTIES constantBufferHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &constantBufferHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &constantBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)));

    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = constantBufferSize;

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_cbvDataBegin)));
}

#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int maxParticles, DirectX::XMFLOAT3 position)
{
	this->emitterData.MaxParticles = maxParticles;
	this->emitterData.Position = position;
    this->emitterData.StartColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    this->emitterData.EndColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    this->emitterData.EmitterIsActive = true;
    this->emitterData.StartSize = 1.0f;
    this->emitterData.EndSize = 0.5f;
    this->emitterData.GravityForce = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);

    particlesData = new Particle[maxParticles];
}

ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::Update(float elapsedTime)
{
    this->emitterData.DeltaTime = elapsedTime;
}

void ParticleSystem::Render(ID3D12GraphicsCommandList* cmdList,
    const std::vector<RenderItem*>& ritems,
    UINT srvDescriptorSize,
    FrameResource* currFrameResource,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = currFrameResource->ObjectCB->Resource();
    auto matCB = currFrameResource->MaterialCB->Resource();

    // For each render item...
    for (size_t i = 0; i < ritems.size(); ++i)
    {
        {
            auto ri = ritems[i];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(srvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, srvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void ParticleSystem::InitializeSystem(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffers[2],
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvUavHeap,
    UINT srvDescriptorSize)
{
    D3D12_RESOURCE_DESC particleBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        sizeof(Particle) * emitterData.MaxParticles,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &particleBufferDesc,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&particleBuffers[0]));

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &particleBufferDesc,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&particleBuffers[1]));


    for (int i = 0; i < 2; i++) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = emitterData.MaxParticles;
        uavDesc.Buffer.StructureByteStride = sizeof(Particle);

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
            srvUavHeap->GetCPUDescriptorHandleForHeapStart(),
            i * 2,
            srvDescriptorSize);

        device->CreateUnorderedAccessView(
            particleBuffers[i].Get(), nullptr, &uavDesc, handle);


        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = emitterData.MaxParticles;
        srvDesc.Buffer.StructureByteStride = sizeof(Particle);
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(
            srvUavHeap->GetCPUDescriptorHandleForHeapStart(),
            i * 2 + 1,
            srvDescriptorSize);

        device->CreateShaderResourceView(
            particleBuffers[i].Get(), &srvDesc, srvHandle);
    }
}

void ParticleSystem::BuildSystemVertexBuffers(
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> &geometries,
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr< ID3D12GraphicsCommandList> cmdList)
{
    struct ParticleVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Size;
    };

    static const int planetCount = 64;
    std::array<ParticleVertex, 64> vertices;
    for (UINT i = 0; i < 64; ++i)
    {

        vertices[i].Pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        vertices[i].Size = DirectX::XMFLOAT2(1.0f, 1.0f);
    }

    std::array<std::uint16_t, 64> indices;
    for (int i = 0; i < 64; ++i)
    {
        indices[i] = i + 1;
    }
    const UINT vbByteSize = (UINT)vertices.size() * sizeof(ParticleVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "particlesGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        cmdList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        cmdList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(ParticleVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount = (UINT)indices.size();
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;

    geo->DrawArgs["points"] = submesh;

    geometries["particlesGeo"] = std::move(geo);
}

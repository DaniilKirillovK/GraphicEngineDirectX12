#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int maxParticles, DirectX::XMFLOAT3 position, int systemID)
{
    if (systemID == 1)
    {
        this->emitterData.MaxParticles = maxParticles;
        this->emitterData.Position = position;
        this->emitterData.StartColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        this->emitterData.EndColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        this->emitterData.EmitterIsActive = true;
        this->emitterData.StartSize = 1.0f;
        this->emitterData.EndSize = 0.5f;
        this->emitterData.GravityForce = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
    }
    else if (systemID == 2)
    {
        this->emitterData.MaxParticles = maxParticles;
        this->emitterData.Position = position;
        this->emitterData.StartColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        this->emitterData.EndColor = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        this->emitterData.EmitterIsActive = true;
        this->emitterData.StartSize = 1.0f;
        this->emitterData.EndSize = 0.5f;
        this->emitterData.GravityForce = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
    }
    else if (systemID == 3)
    {
        this->emitterData.MaxParticles = maxParticles;
        this->emitterData.Position = position;
        this->emitterData.StartColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        this->emitterData.EndColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        this->emitterData.EmitterIsActive = true;
        this->emitterData.StartSize = 1.0f;
        this->emitterData.EndSize = 0.8f;
        this->emitterData.GravityForce = DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f);
    }
    else if (systemID == 4)
    {
        this->emitterData.MaxParticles = maxParticles;
        this->emitterData.Position = position;
        this->emitterData.StartColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        this->emitterData.EndColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        this->emitterData.EmitterIsActive = true;
        this->emitterData.StartSize = 1.0f;
        this->emitterData.EndSize = 0.9f;
        this->emitterData.GravityForce = DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f);
    }
    this->emitterData.SystemID = systemID;
    this->emitterData.TotalTime = 0.0f;
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

void ParticleSystem::RenderGPU(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems, UINT srvDescriptorSize,
    FrameResource* currFrameResource, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap, UINT ParticlesID,
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures, UINT CB1, UINT SRV1,
    Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> texResource)
{
    struct IndirectDrawArgs 
    {
        UINT64 ShaderResourceView0Address;
        UINT64 ShaderResourceView1Address;

        UINT64 ConstantBufferView0Address;
        UINT64 ConstantBufferView1Address;
        UINT64 ConstantBufferView2Address;

        DrawInstancedArgs DrawArgs;
    };

    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
    auto matCB = currFrameResource->MaterialCB->Resource();

    D3D12_INDIRECT_ARGUMENT_DESC arguments[6] = {};

    // Shader Resource View 0 (root parameter index 0)
    arguments[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
    arguments[0].ShaderResourceView.RootParameterIndex = 0;

    // Shader Resource View 1 (root parameter index 1)  
    arguments[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
    arguments[1].ShaderResourceView.RootParameterIndex = 1;

    // Constant Buffer View 0 (root parameter index 2)
    arguments[2].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    arguments[2].ConstantBufferView.RootParameterIndex = 2;

    // Constant Buffer View 1 (root parameter index 3)
    arguments[3].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    arguments[3].ConstantBufferView.RootParameterIndex = 3;

    // Constant Buffer View 2 (root parameter index 4)
    arguments[4].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    arguments[4].ConstantBufferView.RootParameterIndex = 4;

    // Draw command
    arguments[5].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.ByteStride = sizeof(IndirectDrawArgs);
    commandSignatureDesc.NumArgumentDescs = 6;
    commandSignatureDesc.pArgumentDescs = arguments;

    ID3D12CommandSignature* commandSignature;
    device->CreateCommandSignature(&commandSignatureDesc, mRootSignatures["mRootSignatureParticlesGPU"].Get(), IID_PPV_ARGS(&commandSignature));

    IndirectDrawArgs* commands = new IndirectDrawArgs[1];

    // For each render item...
    for (size_t i = 0; i < ritems.size(); ++i)
    {
        {
            auto ri = ritems[i];

            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            commands[i].ShaderResourceView0Address = texResource->GetGPUVirtualAddress();
            commands[i].ShaderResourceView1Address = SRV1;

            commands[i].ConstantBufferView0Address = CB1;
            commands[i].ConstantBufferView1Address = matCBAddress;
            commands[i].ConstantBufferView2Address = 0;

            commands[i].DrawArgs.VertexCountPerInstance = 512;
            commands[i].DrawArgs.InstanceCount = 1;
            commands[i].DrawArgs.StartVertexLocation = 0;
            commands[i].DrawArgs.StartInstanceLocation = i;

            //cmdList->ExecuteIndirect(commandSignature,
            //    1,
            //    );
        }
    }


}

void ParticleSystem::InitializeSystem(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffers[2],
    Microsoft::WRL::ComPtr<ID3D12Resource>& particleArgsBuffer,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvUavHeap,
    UINT srvDescriptorSize, 
    UINT offset)
{
    D3D12_RESOURCE_DESC particleBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        sizeof(Particle) * emitterData.MaxParticles,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    D3D12_RESOURCE_DESC particleArgsBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(DrawInstancedArgs), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &particleBufferDesc,
        D3D12_RESOURCE_STATE_PRESENT,
        nullptr,
        IID_PPV_ARGS(&particleBuffers[0]));

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &particleBufferDesc,
        D3D12_RESOURCE_STATE_PRESENT,
        nullptr,
        IID_PPV_ARGS(&particleBuffers[1]));

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &particleArgsBufferDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&particleArgsBuffer));

    for (int i = 0; i < 2; i++) 
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = emitterData.MaxParticles;
        uavDesc.Buffer.StructureByteStride = sizeof(Particle);

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
            srvUavHeap->GetCPUDescriptorHandleForHeapStart(),
            offset * 5 + i * 2,
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
            offset * 5 + i * 2 + 1,
            srvDescriptorSize);

        device->CreateShaderResourceView(
            particleBuffers[i].Get(), &srvDesc, srvHandle);
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = 1;
    uavDesc.Buffer.StructureByteStride = sizeof(DrawInstancedArgs);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(
        srvUavHeap->GetCPUDescriptorHandleForHeapStart(),
        offset * 5 + 4,
        srvDescriptorSize);

    device->CreateUnorderedAccessView(
        particleArgsBuffer.Get(),    
        nullptr,       
        &uavDesc,      
        uavHandle      
    );
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

    const int particleCount = this->emitterData.MaxParticles;
    std::vector<ParticleVertex> vertices;
    for (UINT i = 0; i < particleCount; ++i)
    {
        ParticleVertex vertex;
        vertex.Pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        vertex.Size = DirectX::XMFLOAT2(1.0f, 1.0f);

        vertices.push_back(vertex);
    }

    std::vector<std::uint16_t> indices;
    for (int i = 0; i < particleCount; ++i)
    {
        int index = i + 1;
        indices.push_back(index);
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

    geo->DrawArgs["points" + std::to_string(emitterData.SystemID)] = submesh;

    geometries["particlesGeo" + std::to_string(emitterData.SystemID)] = std::move(geo);
}

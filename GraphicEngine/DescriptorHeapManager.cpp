#include "DescriptorHeapManager.h"

ExampleDescriptorHeapAllocator DescriptorHeapManager::mSrvHeapAllocator = ExampleDescriptorHeapAllocator();
ExampleDescriptorHeapAllocator DescriptorHeapManager::mSrvHeapAllocator2 = ExampleDescriptorHeapAllocator();
ExampleDescriptorHeapAllocator DescriptorHeapManager::mSrvHeapAllocatorSponza = ExampleDescriptorHeapAllocator();
ExampleDescriptorHeapAllocator DescriptorHeapManager::mSrvHeapAllocatorShadows = ExampleDescriptorHeapAllocator();
ExampleDescriptorHeapAllocator DescriptorHeapManager::mUavHeapAllocator = ExampleDescriptorHeapAllocator();

UINT DescriptorHeapManager::mRtvDescriptorSize = 0;
UINT DescriptorHeapManager::mDsvDescriptorSize = 0;
UINT DescriptorHeapManager::mCbvSrvUavDescriptorSize = 0;
UINT DescriptorHeapManager::mCbvSrvDescriptorSize = 0;

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mRtvHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mDsvHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mSrvHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mUavHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mParticlesSrvUavHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeapManager::mSponzaSrvHeap = Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>();


void DescriptorHeapManager::CreateRtvAndDsvDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 12;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 8;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void DescriptorHeapManager::BuildDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    //
    // Create the SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 75;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

    mSrvHeapAllocator.Create(device.Get(), mSrvHeap.Get());

    //
    // Create particles SRV/UAV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvParticlesHeapDesc = {};
    srvParticlesHeapDesc.NumDescriptors = 40;
    srvParticlesHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvParticlesHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvParticlesHeapDesc, IID_PPV_ARGS(&mParticlesSrvUavHeap)));

    mSrvHeapAllocator2.Create(device.Get(), mParticlesSrvUavHeap.Get());

    //
    // Create sponza SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvSponzaHeapDesc = {};
    srvSponzaHeapDesc.NumDescriptors = 35;
    srvSponzaHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvSponzaHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvSponzaHeapDesc, IID_PPV_ARGS(&mSponzaSrvHeap)));

    mSrvHeapAllocatorSponza.Create(device.Get(), mSponzaSrvHeap.Get());

    BuildShadowMapsDescriptors(device);
}

void DescriptorHeapManager::BuildShadowMapsDescriptors(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    auto srvCpuStart = mSponzaSrvHeap->GetCPUDescriptorHandleForHeapStart();
    auto srvGpuStart = mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart();
    auto dsvCpuStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

    mShadowMap256->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, 24, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, 24, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 2, mDsvDescriptorSize));

    mShadowMap512->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, 25, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, 25, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 3, mDsvDescriptorSize));

    mShadowMap1024->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, 26, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, 26, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 4, mDsvDescriptorSize));

    mShadowMap2048->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, 27, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, 27, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 5, mDsvDescriptorSize));

    srvCpuStart = mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
    srvGpuStart = mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
    dsvCpuStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();

    mShadowMapScene6->BuildDescriptors(
        CD3DX12_CPU_DESCRIPTOR_HANDLE(srvCpuStart, 26, mCbvSrvUavDescriptorSize),
        CD3DX12_GPU_DESCRIPTOR_HANDLE(srvGpuStart, 26, mCbvSrvUavDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvCpuStart, 6, mDsvDescriptorSize));

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSponzaSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(28, mCbvSrvDescriptorSize);

    auto ShadowTex1 = GeometryManager::mTextures["Shadow1Tex"]->Resource;
    auto ShadowTex2 = GeometryManager::mTextures["Shadow2Tex"]->Resource;
    auto ShadowTex3 = GeometryManager::mTextures["Shadow3Tex"]->Resource;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ShadowTex1->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(ShadowTex1.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = ShadowTex2->GetDesc().Format;
    device->CreateShaderResourceView(ShadowTex2.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = ShadowTex3->GetDesc().Format;
    device->CreateShaderResourceView(ShadowTex3.Get(), &srvDesc1, hDescriptor);
}

void DescriptorHeapManager::UploadTextures(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    //
    // Fill out the heap with actual descriptors.
    //
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto MetalTex = GeometryManager::mTextures["MetalTex"]->Resource;
    auto MetalNorm = GeometryManager::mTextures["MetalNorm"]->Resource;
    auto MetalDisplacement = GeometryManager::mTextures["MetalDisplacement"]->Resource;
    auto MetalRoughness = GeometryManager::mTextures["MetalRoughness"]->Resource;
    auto MetalAO = GeometryManager::mTextures["MetalAO"]->Resource;


    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = MetalTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(MetalTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalNorm->GetDesc().Format;
    device->CreateShaderResourceView(MetalNorm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalDisplacement->GetDesc().Format;
    srvDesc1.Texture2D.MipLevels = MetalDisplacement->GetDesc().MipLevels;
    device->CreateShaderResourceView(MetalDisplacement.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalRoughness->GetDesc().Format;
    device->CreateShaderResourceView(MetalRoughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalAO->GetDesc().Format;
    device->CreateShaderResourceView(MetalAO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto StoneTex = GeometryManager::mTextures["StoneTex"]->Resource;
    auto StoneNorm = GeometryManager::mTextures["StoneNorm"]->Resource;
    auto StoneDisplacement = GeometryManager::mTextures["StoneDisplacement"]->Resource;
    auto StoneRoughness = GeometryManager::mTextures["StoneRoughness"]->Resource;
    auto StoneAO = GeometryManager::mTextures["StoneAO"]->Resource;


    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3 = {};
    srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc3.Format = StoneTex->GetDesc().Format;
    srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc3.Texture2D.MostDetailedMip = 0;
    srvDesc3.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(StoneTex.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneNorm->GetDesc().Format;
    device->CreateShaderResourceView(StoneNorm.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneDisplacement->GetDesc().Format;
    srvDesc3.Texture2D.MipLevels = StoneDisplacement->GetDesc().MipLevels;
    device->CreateShaderResourceView(StoneDisplacement.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneRoughness->GetDesc().Format;
    device->CreateShaderResourceView(StoneRoughness.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneAO->GetDesc().Format;
    device->CreateShaderResourceView(StoneAO.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

void DescriptorHeapManager::UploadTextures2(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(17, mCbvSrvDescriptorSize);

    auto PlanetTex = GeometryManager::mTextures["PlanetTex"]->Resource;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PlanetTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PlanetTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto SkyboxTex = GeometryManager::mTextures["SkyboxTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = SkyboxTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = SkyboxTex->GetDesc().MipLevels;
    device->CreateShaderResourceView(SkyboxTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(5, mCbvSrvDescriptorSize);

    auto ParticleTex = GeometryManager::mTextures["ParticleTex"]->Resource;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptorParticles(mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptorParticles.Offset(21, mCbvSrvDescriptorSize);

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(ParticleTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto Particle2Tex = GeometryManager::mTextures["Particle2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(Particle2Tex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto ParticleSmokeTex = GeometryManager::mTextures["ParticleSmokeTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleSmokeTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(ParticleSmokeTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto BonfireTex = GeometryManager::mTextures["BonfireTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = BonfireTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(BonfireTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto GrassTex = GeometryManager::mTextures["GrassTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = GrassTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(GrassTex.Get(), &srvDesc1, hDescriptorParticles);

    auto PBR1Tex = GeometryManager::mTextures["PBR1Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR1Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Norm = GeometryManager::mTextures["PBR1Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR1Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Metallic = GeometryManager::mTextures["PBR1Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR1Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Roughness = GeometryManager::mTextures["PBR1Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR1Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1AO = GeometryManager::mTextures["PBR1AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR1AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Tex = GeometryManager::mTextures["PBR2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR2Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Norm = GeometryManager::mTextures["PBR2Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR2Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Metallic = GeometryManager::mTextures["PBR2Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR2Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Roughness = GeometryManager::mTextures["PBR2Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR2Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2AO = GeometryManager::mTextures["PBR2AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR2AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Tex = GeometryManager::mTextures["PBR3Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR3Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Norm = GeometryManager::mTextures["PBR3Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR3Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Metallic = GeometryManager::mTextures["PBR3Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR3Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Roughness = GeometryManager::mTextures["PBR3Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR3Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3AO = GeometryManager::mTextures["PBR3AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR3AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Tex = GeometryManager::mTextures["PBR4Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR4Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Norm = GeometryManager::mTextures["PBR4Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR4Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Metallic = GeometryManager::mTextures["PBR4Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR4Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Roughness = GeometryManager::mTextures["PBR4Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR4Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4AO = GeometryManager::mTextures["PBR4AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR4AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Tex = GeometryManager::mTextures["PBR5Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR5Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Norm = GeometryManager::mTextures["PBR5Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR5Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Metallic = GeometryManager::mTextures["PBR5Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR5Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Roughness = GeometryManager::mTextures["PBR5Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR5Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5AO = GeometryManager::mTextures["PBR5AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PBR5AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto IrradianceMap = GeometryManager::mTextures["IrradianceMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = IrradianceMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(IrradianceMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PreFilteredEnvMap = GeometryManager::mTextures["PreFilteredEnvMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PreFilteredEnvMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(PreFilteredEnvMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto IntegrationMap = GeometryManager::mTextures["IntegrationMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = IntegrationMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(IntegrationMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor2(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor2.Offset(59, mCbvSrvDescriptorSize);

    auto Terrain2Tex = GeometryManager::mTextures["Terrain2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(Terrain2Tex.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto Terrain2Norm = GeometryManager::mTextures["Terrain2Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(Terrain2Norm.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto Terrain2Height = GeometryManager::mTextures["Terrain2Height"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Height->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(Terrain2Height.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);
}

void DescriptorHeapManager::UploadTextures3(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(54, mCbvSrvDescriptorSize);

    auto TerrainTex = GeometryManager::mTextures["TerrainTex"]->Resource;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = TerrainTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(TerrainTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto TerrainHeight = GeometryManager::mTextures["TerrainHeight"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = TerrainHeight->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = TerrainHeight->GetDesc().MipLevels;
    device->CreateShaderResourceView(TerrainHeight.Get(), &srvDesc1, hDescriptor);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptorParticles(mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptorParticles.Offset(27, mCbvSrvDescriptorSize);

    auto RainTex = GeometryManager::mTextures["RainTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = RainTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = RainTex->GetDesc().MipLevels;
    device->CreateShaderResourceView(RainTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto RainCircleTex = GeometryManager::mTextures["RainCircleTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = RainCircleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = RainCircleTex->GetDesc().MipLevels;
    device->CreateShaderResourceView(RainCircleTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor2(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor2.Offset(62, mCbvSrvDescriptorSize);

    auto HamburgerTex = GeometryManager::mTextures["HamburgerTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerTex->GetDesc().MipLevels;
    device->CreateShaderResourceView(HamburgerTex.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerNorm = GeometryManager::mTextures["HamburgerNorm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerNorm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerNorm->GetDesc().MipLevels;
    device->CreateShaderResourceView(HamburgerNorm.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerMetallic = GeometryManager::mTextures["HamburgerMetallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerMetallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerMetallic->GetDesc().MipLevels;
    device->CreateShaderResourceView(HamburgerMetallic.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerRoughness = GeometryManager::mTextures["HamburgerRoughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerRoughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerRoughness->GetDesc().MipLevels;
    device->CreateShaderResourceView(HamburgerRoughness.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerAO = GeometryManager::mTextures["HamburgerAO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerAO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerAO->GetDesc().MipLevels;
    device->CreateShaderResourceView(HamburgerAO.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);
}

void DescriptorHeapManager::CreateScene3RTV(Microsoft::WRL::ComPtr<ID3D12Device> device)
{

}

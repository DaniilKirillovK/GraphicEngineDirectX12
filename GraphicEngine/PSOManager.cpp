#include "PSOManager.h"

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> PSOManager::mPSOs = std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>>();

void PSOManager::BuildPSOs(std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures,
    std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> inputLayouts,
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> shaders,
    DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthStencilFormat,
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultForwardPsoDesc;

    //
    // PSO for default forward rendering
    //
    ZeroMemory(&defaultForwardPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defaultForwardPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size()};
    defaultForwardPsoDesc.pRootSignature = rootSignatures["mRootSignatureDefaultForward"].Get();
    defaultForwardPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardVS"]->GetBufferPointer()),
        shaders["forwardVS"]->GetBufferSize()
    };
    defaultForwardPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardPS"]->GetBufferPointer()),
        shaders["forwardPS"]->GetBufferSize()
    };
    defaultForwardPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    defaultForwardPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.SampleMask = UINT_MAX;
    defaultForwardPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    defaultForwardPsoDesc.NumRenderTargets = 1;
    defaultForwardPsoDesc.RTVFormats[0] = backBufferFormat;
    defaultForwardPsoDesc.SampleDesc.Count = 1;
    defaultForwardPsoDesc.SampleDesc.Quality = 0;
    defaultForwardPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defaultForwardPsoDesc, IID_PPV_ARGS(&mPSOs["forwardDefault"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultForwardFrustumCullingPsoDesc;
    //
    // PSO for default forward rendering with frustum culling
    //
    defaultForwardFrustumCullingPsoDesc = defaultForwardPsoDesc;
    defaultForwardFrustumCullingPsoDesc.pRootSignature = rootSignatures["mRootSignatureDefaultForwardFrustumCulling"].Get();
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defaultForwardFrustumCullingPsoDesc, IID_PPV_ARGS(&mPSOs["forwardDefaultFrustumCulling"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size()};
    opaquePsoDesc.pRootSignature = rootSignatures["mRootSignature"].Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["tessVS"]->GetBufferPointer()),
        shaders["tessVS"]->GetBufferSize()
    };
    opaquePsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["tessHS"]->GetBufferPointer()),
        shaders["tessHS"]->GetBufferSize()
    };
    opaquePsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["tessDS"]->GetBufferPointer()),
        shaders["tessDS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["tessPS"]->GetBufferPointer()),
        shaders["tessPS"]->GetBufferSize()
    };
    opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaquePsoDesc.SampleMask = UINT_MAX;
    opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    opaquePsoDesc.NumRenderTargets = 4;
    opaquePsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaquePsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaquePsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaquePsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaquePsoDesc.SampleDesc.Count = 1;
    opaquePsoDesc.SampleDesc.Quality = 0;
    opaquePsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaqueSolid"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC moreSamplersPsoDesc = opaquePsoDesc;
    moreSamplersPsoDesc.pRootSignature = rootSignatures["mRootSignatureMoreSamplers"].Get();
    moreSamplersPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["moreSamplersVS"]->GetBufferPointer()),
        shaders["moreSamplersVS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["moreSamplersHS"]->GetBufferPointer()),
        shaders["moreSamplersHS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["moreSamplersDS"]->GetBufferPointer()),
        shaders["moreSamplersDS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["moreSamplersPS"]->GetBufferPointer()),
        shaders["moreSamplersPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&moreSamplersPsoDesc, IID_PPV_ARGS(&mPSOs["moreSamplers"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc;

    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaqueWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaqueWireframePsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    opaqueWireframePsoDesc.pRootSignature = rootSignatures["mRootSignature"].Get();
    opaqueWireframePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["tessVS"]->GetBufferPointer()),
        shaders["tessVS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["tessHS"]->GetBufferPointer()),
        shaders["tessHS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["tessDS"]->GetBufferPointer()),
        shaders["tessDS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["tessPS"]->GetBufferPointer()),
        shaders["tessPS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    opaqueWireframePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    opaqueWireframePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    opaqueWireframePsoDesc.SampleMask = UINT_MAX;
    opaqueWireframePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    opaqueWireframePsoDesc.NumRenderTargets = 4;
    opaqueWireframePsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaqueWireframePsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaqueWireframePsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaqueWireframePsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    opaqueWireframePsoDesc.SampleDesc.Count = 1;
    opaqueWireframePsoDesc.SampleDesc.Quality = 0;
    opaqueWireframePsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaqueWireframe"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDescPixel;
    ZeroMemory(&opaquePsoDescPixel, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDescPixel = opaquePsoDesc;
    opaquePsoDescPixel.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PSPixel"]->GetBufferPointer()),
        shaders["PSPixel"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDescPixel, IID_PPV_ARGS(&mPSOs["opaquePixel"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc;
    //
    // PSO for debug layer
    //
    ZeroMemory(&debugPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    debugPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    debugPsoDesc.pRootSignature = rootSignatures["mRootSignatureDebug"].Get();
    debugPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["debugVS"]->GetBufferPointer()),
        shaders["debugVS"]->GetBufferSize()
    };
    debugPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["debugPS"]->GetBufferPointer()),
        shaders["debugPS"]->GetBufferSize()
    };
    debugPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    debugPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    debugPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    debugPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    debugPsoDesc.SampleMask = UINT_MAX;
    debugPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    debugPsoDesc.NumRenderTargets = 4;
    debugPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    debugPsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    debugPsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    debugPsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    debugPsoDesc.SampleDesc.Count = 1;
    debugPsoDesc.SampleDesc.Quality = 0;
    debugPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["debug"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightPsoDesc;
    ZeroMemory(&lightPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    lightPsoDesc.InputLayout = { inputLayouts["InputLayoutLight"].data(), (UINT)inputLayouts["InputLayoutLight"].size() };
    lightPsoDesc.pRootSignature = rootSignatures["mRootSignatureLight"].Get();
    lightPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["DeferredVSLighting"]->GetBufferPointer()),
        shaders["DeferredVSLighting"]->GetBufferSize()
    };
    lightPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["DeferredPSLighting"]->GetBufferPointer()),
        shaders["DeferredPSLighting"]->GetBufferSize()
    };
    lightPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    lightPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    lightPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    lightPsoDesc.DepthStencilState.DepthEnable = FALSE;
    lightPsoDesc.DepthStencilState.StencilEnable = FALSE;
    lightPsoDesc.SampleMask = UINT_MAX;
    lightPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    lightPsoDesc.NumRenderTargets = 1;
    lightPsoDesc.RTVFormats[0] = backBufferFormat;
    lightPsoDesc.SampleDesc.Count = 1;
    lightPsoDesc.SampleDesc.Quality = 0;
    lightPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&lightPsoDesc, IID_PPV_ARGS(&mPSOs["deferredLighting"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightMoreLightPsoDesc;
    ZeroMemory(&lightMoreLightPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    lightMoreLightPsoDesc = lightPsoDesc;
    lightMoreLightPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["DefferedVSLightingMoreLight"]->GetBufferPointer()),
        shaders["DefferedVSLightingMoreLight"]->GetBufferSize()
    };
    lightMoreLightPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["DefferedPSLightingMoreLight"]->GetBufferPointer()),
        shaders["DefferedPSLightingMoreLight"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&lightMoreLightPsoDesc, IID_PPV_ARGS(&mPSOs["defferedLightingMoreLight"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedLightingNoPosPsoDesc;
    ZeroMemory(&defferedLightingNoPosPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedLightingNoPosPsoDesc = lightPsoDesc;
    defferedLightingNoPosPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["DefferedLightingNoPosRT_VS"]->GetBufferPointer()),
        shaders["DefferedLightingNoPosRT_VS"]->GetBufferSize()
    };
    defferedLightingNoPosPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["DefferedLightingNoPosRT_PS"]->GetBufferPointer()),
        shaders["DefferedLightingNoPosRT_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedLightingNoPosPsoDesc, IID_PPV_ARGS(&mPSOs["deferredLightingNoPos"])));

    //
    // PSO for billboard sprites
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC billboardSpritePsoDesc;
    ZeroMemory(&billboardSpritePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    billboardSpritePsoDesc.pRootSignature = rootSignatures["mRootSignatureBillboard"].Get();
    billboardSpritePsoDesc.InputLayout = { inputLayouts["BillboardSpriteInputLayout"].data(), (UINT)inputLayouts["BillboardSpriteInputLayout"].size() };
    billboardSpritePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["billboardSpriteVS"]->GetBufferPointer()),
        shaders["billboardSpriteVS"]->GetBufferSize()
    };
    billboardSpritePsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["billboardSpriteGS"]->GetBufferPointer()),
        shaders["billboardSpriteGS"]->GetBufferSize()
    };
    billboardSpritePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["billboardSpritePS"]->GetBufferPointer()),
        shaders["billboardSpritePS"]->GetBufferSize()
    };
    billboardSpritePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    billboardSpritePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    billboardSpritePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    billboardSpritePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    billboardSpritePsoDesc.SampleMask = UINT_MAX;
    billboardSpritePsoDesc.NumRenderTargets = 4;
    billboardSpritePsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    billboardSpritePsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    billboardSpritePsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    billboardSpritePsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    billboardSpritePsoDesc.SampleDesc.Count = 1;
    billboardSpritePsoDesc.SampleDesc.Quality = 0;
    billboardSpritePsoDesc.DSVFormat = depthStencilFormat;
    billboardSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    billboardSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&billboardSpritePsoDesc, IID_PPV_ARGS(&mPSOs["billboardSprites"])));

    //
    // PSO for particles compute
    //
    D3D12_COMPUTE_PIPELINE_STATE_DESC particlesComputePsoDesc;
    ZeroMemory(&particlesComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

    particlesComputePsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesCompute"].Get();
    particlesComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(shaders["particlesCS"]->GetBufferPointer()),
        shaders["particlesCS"]->GetBufferSize()
    };
    particlesComputePsoDesc.NodeMask = 0;
    particlesComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&particlesComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeParticles"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC particlesArgsComputePsoDesc;
    ZeroMemory(&particlesArgsComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

    particlesArgsComputePsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesArgsCompute"].Get();
    particlesArgsComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(shaders["particlesArgsCS"]->GetBufferPointer()),
        shaders["particlesArgsCS"]->GetBufferSize()
    };
    particlesArgsComputePsoDesc.NodeMask = 0;
    particlesArgsComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&particlesArgsComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeParticlesArgs"])));


    //
    // PSO for particles render
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesPsoDesc;
    ZeroMemory(&particlesPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesPsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesRender"].Get();
    particlesPsoDesc.InputLayout = { inputLayouts["ParticlesInputLayout"].data(), (UINT)inputLayouts["ParticlesInputLayout"].size() };
    particlesPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesVS"]->GetBufferPointer()),
        shaders["particlesVS"]->GetBufferSize()
    };
    particlesPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesGS"]->GetBufferPointer()),
        shaders["particlesGS"]->GetBufferSize()
    };
    particlesPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesPS"]->GetBufferPointer()),
        shaders["particlesPS"]->GetBufferSize()
    };

    particlesPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    particlesPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    particlesPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    particlesPsoDesc.RasterizerState.DepthClipEnable = TRUE;

    particlesPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    particlesPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    particlesPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    particlesPsoDesc.SampleMask = UINT_MAX;
    particlesPsoDesc.NumRenderTargets = 4;
    particlesPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    particlesPsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    particlesPsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    particlesPsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    particlesPsoDesc.SampleDesc.Count = 1;
    particlesPsoDesc.SampleDesc.Quality = 0;
    particlesPsoDesc.DSVFormat = depthStencilFormat;
    particlesPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesPsoDesc, IID_PPV_ARGS(&mPSOs["renderParticles"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesForwardPsoDesc;
    ZeroMemory(&particlesForwardPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesForwardPsoDesc = particlesPsoDesc;
    particlesForwardPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardVS"]->GetBufferPointer()),
        shaders["particlesForwardVS"]->GetBufferSize()
    };
    particlesForwardPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardGS"]->GetBufferPointer()),
        shaders["particlesForwardGS"]->GetBufferSize()
    };
    particlesForwardPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardPS"]->GetBufferPointer()),
        shaders["particlesForwardPS"]->GetBufferSize()
    };
    particlesForwardPsoDesc.NumRenderTargets = 1;
    particlesForwardPsoDesc.RTVFormats[0] = backBufferFormat;
    particlesForwardPsoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
    particlesForwardPsoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
    particlesForwardPsoDesc.RTVFormats[3] = DXGI_FORMAT_UNKNOWN;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesForwardPsoDesc, IID_PPV_ARGS(&mPSOs["renderParticlesForward"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesForwardVertexLightingPsoDesc;
    ZeroMemory(&particlesForwardVertexLightingPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesForwardVertexLightingPsoDesc = particlesForwardPsoDesc;
    particlesForwardVertexLightingPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardVertexLightingVS"]->GetBufferPointer()),
        shaders["particlesForwardVertexLightingVS"]->GetBufferSize()
    };
    particlesForwardVertexLightingPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardVertexLightingGS"]->GetBufferPointer()),
        shaders["particlesForwardVertexLightingGS"]->GetBufferSize()
    };
    particlesForwardVertexLightingPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardVertexLightingPS"]->GetBufferPointer()),
        shaders["particlesForwardVertexLightingPS"]->GetBufferSize()
    };

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesForwardVertexLightingPsoDesc, IID_PPV_ARGS(&mPSOs["renderParticlesForwardVertexLighting"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesForwardGPUPsoDesc;
    ZeroMemory(&particlesForwardGPUPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesForwardGPUPsoDesc = particlesForwardPsoDesc;
    particlesForwardGPUPsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesGPU"].Get();
    particlesForwardGPUPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardVS"]->GetBufferPointer()),
        shaders["particlesForwardVS"]->GetBufferSize()
    };
    particlesForwardGPUPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardGS"]->GetBufferPointer()),
        shaders["particlesForwardGS"]->GetBufferSize()
    };
    particlesForwardGPUPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesForwardPS"]->GetBufferPointer()),
        shaders["particlesForwardPS"]->GetBufferSize()
    };

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesForwardVertexLightingPsoDesc, IID_PPV_ARGS(&mPSOs["renderParticlesForwardGPU"])));


    // Post Processing mPSOs
    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingGCPsoDesc;
    ZeroMemory(&postProcessingGCPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    postProcessingGCPsoDesc.InputLayout = { inputLayouts["PostProcessingInputLayout"].data(), (UINT)inputLayouts["PostProcessingInputLayout"].size() };
    postProcessingGCPsoDesc.pRootSignature = rootSignatures["mRootSignaturePostProcessing"].Get();
    postProcessingGCPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingVS"]->GetBufferPointer()),
        shaders["PostProcessingVS"]->GetBufferSize()
    };
    postProcessingGCPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_GC"]->GetBufferPointer()),
        shaders["PostProcessingPS_GC"]->GetBufferSize()
    };
    postProcessingGCPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    postProcessingGCPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    postProcessingGCPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    postProcessingGCPsoDesc.DepthStencilState.DepthEnable = FALSE;
    postProcessingGCPsoDesc.DepthStencilState.StencilEnable = FALSE;
    postProcessingGCPsoDesc.SampleMask = UINT_MAX;
    postProcessingGCPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    postProcessingGCPsoDesc.NumRenderTargets = 1;
    postProcessingGCPsoDesc.RTVFormats[0] = backBufferFormat;
    postProcessingGCPsoDesc.SampleDesc.Count = 1;
    postProcessingGCPsoDesc.SampleDesc.Quality = 0;
    postProcessingGCPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingGCPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_GC"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingGBPsoDesc = postProcessingGCPsoDesc;
    postProcessingGBPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_GB"]->GetBufferPointer()),
        shaders["PostProcessingPS_GB"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingGBPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_GB"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingCAPsoDesc = postProcessingGCPsoDesc;
    postProcessingCAPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_CA"]->GetBufferPointer()),
        shaders["PostProcessingPS_CA"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingCAPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_CA"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingVigPsoDesc = postProcessingGCPsoDesc;
    postProcessingVigPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_Vig"]->GetBufferPointer()),
        shaders["PostProcessingPS_Vig"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingVigPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Vig"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingNoisePsoDesc = postProcessingGCPsoDesc;
    postProcessingNoisePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_Noise"]->GetBufferPointer()),
        shaders["PostProcessingPS_Noise"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingNoisePsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Noise"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingDefaultPsoDesc = postProcessingGCPsoDesc;
    postProcessingDefaultPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PostProcessingPS_Default"]->GetBufferPointer()),
        shaders["PostProcessingPS_Default"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingDefaultPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Default"])));

    //
   // PSO for noise compute
   //
    D3D12_COMPUTE_PIPELINE_STATE_DESC noiseComputePsoDesc;
    ZeroMemory(&noiseComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

    noiseComputePsoDesc.pRootSignature = rootSignatures["mRootSignatureComputeNoise"].Get();
    noiseComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(shaders["noiseCS"]->GetBufferPointer()),
        shaders["noiseCS"]->GetBufferSize()
    };
    noiseComputePsoDesc.NodeMask = 0;
    noiseComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&noiseComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeNoise"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC forwardRTPsoDesc;
    ZeroMemory(&forwardRTPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    forwardRTPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    forwardRTPsoDesc.pRootSignature = rootSignatures["mRootSignatureRT"].Get();
    forwardRTPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardRT_VS"]->GetBufferPointer()),
        shaders["forwardRT_VS"]->GetBufferSize()
    };
    forwardRTPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardRT_PS"]->GetBufferPointer()),
        shaders["forwardRT_PS"]->GetBufferSize()
    };
    forwardRTPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    forwardRTPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.SampleMask = UINT_MAX;
    forwardRTPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    forwardRTPsoDesc.NumRenderTargets = 1;
    forwardRTPsoDesc.RTVFormats[0] = backBufferFormat;
    forwardRTPsoDesc.SampleDesc.Count = 1;
    forwardRTPsoDesc.SampleDesc.Quality = 0;
    forwardRTPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&forwardRTPsoDesc, IID_PPV_ARGS(&mPSOs["forwardRT"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC forwardRTMoreLightPsoDesc;
    ZeroMemory(&forwardRTMoreLightPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    forwardRTMoreLightPsoDesc = forwardRTPsoDesc;
    forwardRTMoreLightPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardRTMoreLight_VS"]->GetBufferPointer()),
        shaders["forwardRTMoreLight_VS"]->GetBufferSize()
    };
    forwardRTMoreLightPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["forwardRTMoreLight_PS"]->GetBufferPointer()),
        shaders["forwardRTMoreLight_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&forwardRTMoreLightPsoDesc, IID_PPV_ARGS(&mPSOs["forwardRTMoreLight"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedRTPsoDesc;
    ZeroMemory(&defferedRTPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedRTPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    defferedRTPsoDesc.pRootSignature = rootSignatures["mRootSignatureRT"].Get();
    defferedRTPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedRT_VS"]->GetBufferPointer()),
        shaders["defferedRT_VS"]->GetBufferSize()
    };
    defferedRTPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedRT_PS"]->GetBufferPointer()),
        shaders["defferedRT_PS"]->GetBufferSize()
    };
    defferedRTPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    defferedRTPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    defferedRTPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    defferedRTPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    defferedRTPsoDesc.SampleMask = UINT_MAX;
    defferedRTPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    defferedRTPsoDesc.NumRenderTargets = 4;
    defferedRTPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    defferedRTPsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    defferedRTPsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    defferedRTPsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    defferedRTPsoDesc.SampleDesc.Count = 1;
    defferedRTPsoDesc.SampleDesc.Quality = 0;
    defferedRTPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedRTPsoDesc, IID_PPV_ARGS(&mPSOs["defferedRT"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedRTNoPosPsoDesc;
    ZeroMemory(&defferedRTNoPosPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedRTNoPosPsoDesc = defferedRTPsoDesc;
    defferedRTNoPosPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedNoPosRT_VS"]->GetBufferPointer()),
        shaders["defferedNoPosRT_VS"]->GetBufferSize()
    };
    defferedRTNoPosPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedNoPosRT_PS"]->GetBufferPointer()),
        shaders["defferedNoPosRT_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedRTNoPosPsoDesc, IID_PPV_ARGS(&mPSOs["defferedRTNoPos"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC decalsTessPsoDesc;
    ZeroMemory(&decalsTessPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    decalsTessPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    decalsTessPsoDesc.pRootSignature = rootSignatures["mRootSignatureTess"].Get();
    decalsTessPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["decalsTessVS"]->GetBufferPointer()),
        shaders["decalsTessVS"]->GetBufferSize()
    };
    decalsTessPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["decalsTessHS"]->GetBufferPointer()),
        shaders["decalsTessHS"]->GetBufferSize()
    };
    decalsTessPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["decalsTessDS"]->GetBufferPointer()),
        shaders["decalsTessDS"]->GetBufferSize()
    };
    decalsTessPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["decalsTessPS"]->GetBufferPointer()),
        shaders["decalsTessPS"]->GetBufferSize()
    };
    decalsTessPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    decalsTessPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    decalsTessPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    decalsTessPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    decalsTessPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    decalsTessPsoDesc.SampleMask = UINT_MAX;
    decalsTessPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    decalsTessPsoDesc.NumRenderTargets = 4;
    decalsTessPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    decalsTessPsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    decalsTessPsoDesc.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    decalsTessPsoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    decalsTessPsoDesc.SampleDesc.Count = 1;
    decalsTessPsoDesc.SampleDesc.Quality = 0;
    decalsTessPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&decalsTessPsoDesc, IID_PPV_ARGS(&mPSOs["decalsTess"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC decalsTessWireframePsoDesc;
    ZeroMemory(&decalsTessWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    decalsTessWireframePsoDesc = decalsTessPsoDesc;
    decalsTessWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&decalsTessWireframePsoDesc, IID_PPV_ARGS(&mPSOs["decalsTessWireframe"])));

    //
    // PSO for shadow map pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc;
    ZeroMemory(&smapPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    smapPsoDesc = forwardRTPsoDesc;
    smapPsoDesc.RasterizerState.DepthBias = 100000;
    smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    smapPsoDesc.pRootSignature = rootSignatures["mRootSignatureShadows"].Get();
    smapPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowVS"]->GetBufferPointer()),
        shaders["shadowVS"]->GetBufferSize()
    };
    smapPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowPS"]->GetBufferPointer()),
        shaders["shadowPS"]->GetBufferSize()
    };

    // Shadow map pass does not have a render target.
    smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapPsoDesc.NumRenderTargets = 0;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&mPSOs["shadowPSO"])));

    //
    // PSO for particles shadow map pass.
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapParticlesPsoDesc;
    ZeroMemory(&smapParticlesPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    smapParticlesPsoDesc = forwardRTPsoDesc;
    smapParticlesPsoDesc.InputLayout = { inputLayouts["ParticlesInputLayout"].data(), (UINT)inputLayouts["ParticlesInputLayout"].size() };
    smapParticlesPsoDesc.RasterizerState.DepthBias = 100000;
    smapParticlesPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    smapParticlesPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
    smapParticlesPsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesRender"].Get();
    smapParticlesPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowParticlesVS"]->GetBufferPointer()),
        shaders["shadowParticlesVS"]->GetBufferSize()
    };
    smapParticlesPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowParticlesGS"]->GetBufferPointer()),
        shaders["shadowParticlesGS"]->GetBufferSize()
    };
    smapParticlesPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowParticlesPS"]->GetBufferPointer()),
        shaders["shadowParticlesPS"]->GetBufferSize()
    };

    // Shadow map pass does not have a render target.
    smapParticlesPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
    smapParticlesPsoDesc.NumRenderTargets = 0;
    smapParticlesPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapParticlesPsoDesc, IID_PPV_ARGS(&mPSOs["shadowParticlesPSO"])));

    //
    // PSO for shadows forward pass
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapForwardPsoDesc;
    ZeroMemory(&smapForwardPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    smapForwardPsoDesc = forwardRTPsoDesc;
    smapForwardPsoDesc.pRootSignature = rootSignatures["mRootSignatureShadowsForward"].Get();
    smapForwardPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["cascadedShadowsForwardVS"]->GetBufferPointer()),
        shaders["cascadedShadowsForwardVS"]->GetBufferSize()
    };
    smapForwardPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["cascadedShadowsForwardPS"]->GetBufferPointer()),
        shaders["cascadedShadowsForwardPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapForwardPsoDesc, IID_PPV_ARGS(&mPSOs["shadowForwardPSO"])));

    //
    // PSO for shadows forward pass with particles
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC smapForwardParticlesPsoDesc;
    ZeroMemory(&smapForwardParticlesPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    smapForwardParticlesPsoDesc = forwardRTPsoDesc;
    smapForwardParticlesPsoDesc.pRootSignature = rootSignatures["mRootSignatureShadowsParticlesForward"].Get();
    smapForwardParticlesPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowsForwardVS"]->GetBufferPointer()),
        shaders["shadowsForwardVS"]->GetBufferSize()
    };
    smapForwardParticlesPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["shadowsForwardPS"]->GetBufferPointer()),
        shaders["shadowsForwardPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&smapForwardParticlesPsoDesc, IID_PPV_ARGS(&mPSOs["shadowForwardParticlesPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugGeometryPsoDesc;
    ZeroMemory(&debugGeometryPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    debugGeometryPsoDesc.InputLayout = { inputLayouts["DebugInputLayout"].data(), (UINT)inputLayouts["DebugInputLayout"].size() };
    debugGeometryPsoDesc.pRootSignature = rootSignatures["mRootSignatureDebugGeometry"].Get();
    debugGeometryPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["debugGeometryVS"]->GetBufferPointer()),
        shaders["debugGeometryVS"]->GetBufferSize()
    };
    debugGeometryPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["debugGeometryPS"]->GetBufferPointer()),
        shaders["debugGeometryPS"]->GetBufferSize()
    };
    debugGeometryPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    debugGeometryPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    debugGeometryPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    debugGeometryPsoDesc.DepthStencilState.DepthEnable = TRUE;
    debugGeometryPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    debugGeometryPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    debugGeometryPsoDesc.DepthStencilState.StencilEnable = FALSE;
    debugGeometryPsoDesc.SampleMask = UINT_MAX;
    debugGeometryPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    debugGeometryPsoDesc.NumRenderTargets = 1;
    debugGeometryPsoDesc.RTVFormats[0] = backBufferFormat;
    debugGeometryPsoDesc.SampleDesc.Count = 1;
    debugGeometryPsoDesc.SampleDesc.Quality = 0;
    debugGeometryPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugGeometryPsoDesc, IID_PPV_ARGS(&mPSOs["debugGeometry"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedPointSpotLightPso;
    ZeroMemory(&defferedPointSpotLightPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedPointSpotLightPso.InputLayout = { inputLayouts["DebugInputLayout"].data(), (UINT)inputLayouts["DebugInputLayout"].size() };
    defferedPointSpotLightPso.pRootSignature = rootSignatures["mRootSignatureDefferedPointSpotDirectional"].Get();
    defferedPointSpotLightPso.VS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedPointSpotVS"]->GetBufferPointer()),
        shaders["defferedPointSpotVS"]->GetBufferSize()
    };
    defferedPointSpotLightPso.PS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedPointSpotPS"]->GetBufferPointer()),
        shaders["defferedPointSpotPS"]->GetBufferSize()
    };
    defferedPointSpotLightPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    defferedPointSpotLightPso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    defferedPointSpotLightPso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    defferedPointSpotLightPso.BlendState.IndependentBlendEnable = FALSE;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].BlendEnable = TRUE;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    defferedPointSpotLightPso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    defferedPointSpotLightPso.DepthStencilState.DepthEnable = FALSE;
    defferedPointSpotLightPso.DepthStencilState.StencilEnable = FALSE;
    defferedPointSpotLightPso.SampleMask = UINT_MAX;
    defferedPointSpotLightPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    defferedPointSpotLightPso.NumRenderTargets = 1;
    defferedPointSpotLightPso.RTVFormats[0] = backBufferFormat;
    defferedPointSpotLightPso.SampleDesc.Count = 1;
    defferedPointSpotLightPso.SampleDesc.Quality = 0;
    defferedPointSpotLightPso.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedPointSpotLightPso, IID_PPV_ARGS(&mPSOs["defferedPointSpot"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedPointSpotLightMoreLightPso;
    ZeroMemory(&defferedPointSpotLightMoreLightPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedPointSpotLightMoreLightPso = defferedPointSpotLightPso;
    defferedPointSpotLightMoreLightPso.pRootSignature = rootSignatures["mRootSignatureMoreLight"].Get();
    defferedPointSpotLightMoreLightPso.VS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedPointSpotMoreLightVS"]->GetBufferPointer()),
        shaders["defferedPointSpotMoreLightVS"]->GetBufferSize()
    };
    defferedPointSpotLightMoreLightPso.PS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedPointSpotMoreLightPS"]->GetBufferPointer()),
        shaders["defferedPointSpotMoreLightPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedPointSpotLightMoreLightPso, IID_PPV_ARGS(&mPSOs["defferedPointSpotMoreLight"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defferedDirectionalLightPso;
    ZeroMemory(&defferedDirectionalLightPso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defferedDirectionalLightPso = defferedPointSpotLightPso;
    defferedDirectionalLightPso.InputLayout = { inputLayouts["InputLayoutLight"].data(), (UINT)inputLayouts["InputLayoutLight"].size() };
    defferedDirectionalLightPso.pRootSignature = rootSignatures["mRootSignatureLight"].Get();
    defferedDirectionalLightPso.VS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedDirectionalVS"]->GetBufferPointer()),
        shaders["defferedDirectionalVS"]->GetBufferSize()
    };
    defferedDirectionalLightPso.PS =
    {
        reinterpret_cast<BYTE*>(shaders["defferedDirectionalPS"]->GetBufferPointer()),
        shaders["defferedDirectionalPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defferedDirectionalLightPso, IID_PPV_ARGS(&mPSOs["defferedDirectional"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC skyboxPsoDesc;
    ZeroMemory(&skyboxPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    skyboxPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    skyboxPsoDesc.pRootSignature = rootSignatures["mRootSignatureSkyBox"].Get();
    skyboxPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["skyboxVS"]->GetBufferPointer()),
        shaders["skyboxVS"]->GetBufferSize()
    };
    skyboxPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["skyboxPS"]->GetBufferPointer()),
        shaders["skyboxPS"]->GetBufferSize()
    };
    skyboxPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    skyboxPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    skyboxPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    skyboxPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    skyboxPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    skyboxPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    skyboxPsoDesc.SampleMask = UINT_MAX;
    skyboxPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    skyboxPsoDesc.NumRenderTargets = 1;
    skyboxPsoDesc.RTVFormats[0] = backBufferFormat;
    skyboxPsoDesc.SampleDesc.Count = 1;
    skyboxPsoDesc.SampleDesc.Quality = 0;
    skyboxPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&skyboxPsoDesc, IID_PPV_ARGS(&mPSOs["skyboxPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PBRPsoDesc;
    ZeroMemory(&PBRPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    PBRPsoDesc = forwardRTPsoDesc;
    PBRPsoDesc.pRootSignature = rootSignatures["mRootSignaturePBR"].Get();
    PBRPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["PBR_VS"]->GetBufferPointer()),
        shaders["PBR_VS"]->GetBufferSize()
    };
    PBRPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["PBR_PS"]->GetBufferPointer()),
        shaders["PBR_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&PBRPsoDesc, IID_PPV_ARGS(&mPSOs["PBRPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC StandartPBRPsoDesc;
    ZeroMemory(&StandartPBRPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    StandartPBRPsoDesc = PBRPsoDesc;
    StandartPBRPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["StandartPBR_VS"]->GetBufferPointer()),
        shaders["StandartPBR_VS"]->GetBufferSize()
    };
    StandartPBRPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["StandartPBR_PS"]->GetBufferPointer()),
        shaders["StandartPBR_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&StandartPBRPsoDesc, IID_PPV_ARGS(&mPSOs["StandartPBRPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC IBLPsoDesc;
    ZeroMemory(&IBLPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    IBLPsoDesc = forwardRTPsoDesc;
    IBLPsoDesc.pRootSignature = rootSignatures["mRootSignaturePBR"].Get();
    IBLPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["IBL_VS"]->GetBufferPointer()),
        shaders["IBL_VS"]->GetBufferSize()
    };
    IBLPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["IBL_PS"]->GetBufferPointer()),
        shaders["IBL_PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&IBLPsoDesc, IID_PPV_ARGS(&mPSOs["IBLPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC RMDemoPsoDesc;
    ZeroMemory(&RMDemoPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    RMDemoPsoDesc = IBLPsoDesc;
    RMDemoPsoDesc.pRootSignature = rootSignatures["mRootSignatureRMDemo"].Get();
    RMDemoPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["RMDemoVS"]->GetBufferPointer()),
        shaders["RMDemoVS"]->GetBufferSize()
    };
    RMDemoPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["RMDemoPS"]->GetBufferPointer()),
        shaders["RMDemoPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&RMDemoPsoDesc, IID_PPV_ARGS(&mPSOs["RMDemoPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC heightMapPsoDesc;
    ZeroMemory(&heightMapPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    heightMapPsoDesc.InputLayout = { inputLayouts["InputLayout"].data(), (UINT)inputLayouts["InputLayout"].size() };
    heightMapPsoDesc.pRootSignature = rootSignatures["mRootSignatureHeightMap"].Get();
    heightMapPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["HeightMapVS"]->GetBufferPointer()),
        shaders["HeightMapVS"]->GetBufferSize()
    };
    heightMapPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["HeightMapHS"]->GetBufferPointer()),
        shaders["HeightMapHS"]->GetBufferSize()
    };
    heightMapPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["HeightMapDS"]->GetBufferPointer()),
        shaders["HeightMapDS"]->GetBufferSize()
    };
    heightMapPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["HeightMapPS"]->GetBufferPointer()),
        shaders["HeightMapPS"]->GetBufferSize()
    };
    heightMapPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    heightMapPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    heightMapPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    heightMapPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    heightMapPsoDesc.SampleMask = UINT_MAX;
    heightMapPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    heightMapPsoDesc.NumRenderTargets = 1;
    heightMapPsoDesc.RTVFormats[0] = backBufferFormat;
    heightMapPsoDesc.SampleDesc.Count = 1;
    heightMapPsoDesc.SampleDesc.Quality = 0;
    heightMapPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&heightMapPsoDesc, IID_PPV_ARGS(&mPSOs["heightMapSolidPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC heightMapWireframePsoDesc;
    ZeroMemory(&heightMapWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    heightMapWireframePsoDesc = heightMapPsoDesc;
    heightMapWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&heightMapWireframePsoDesc, IID_PPV_ARGS(&mPSOs["heightMapWireframePSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC scene13ObjectsPsoDesc;
    ZeroMemory(&scene13ObjectsPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    scene13ObjectsPsoDesc = RMDemoPsoDesc;
    scene13ObjectsPsoDesc.pRootSignature = rootSignatures["mRootSignatureObjectsScene13"].Get();
    scene13ObjectsPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["Scene13VS"]->GetBufferPointer()),
        shaders["Scene13VS"]->GetBufferSize()
    };
    scene13ObjectsPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["Scene13PS"]->GetBufferPointer()),
        shaders["Scene13PS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&scene13ObjectsPsoDesc, IID_PPV_ARGS(&mPSOs["Scene13ObjectsPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC scene13OctreePsoDesc;
    ZeroMemory(&scene13OctreePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    scene13OctreePsoDesc = scene13ObjectsPsoDesc;
    scene13OctreePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    scene13OctreePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    scene13OctreePsoDesc.InputLayout = { inputLayouts["DebugInputLayout"].data(), (UINT)inputLayouts["DebugInputLayout"].size() };
    scene13OctreePsoDesc.pRootSignature = rootSignatures["mRootSignatureOctreeScene13"].Get();
    scene13OctreePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["Scene13OcTreeVS"]->GetBufferPointer()),
        shaders["Scene13OcTreeVS"]->GetBufferSize()
    };
    scene13OctreePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["Scene13OcTreePS"]->GetBufferPointer()),
        shaders["Scene13OcTreePS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&scene13OctreePsoDesc, IID_PPV_ARGS(&mPSOs["Scene13OctreePSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesRainPsoDesc;
    ZeroMemory(&particlesRainPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesRainPsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesRain"].Get();
    particlesRainPsoDesc.InputLayout = { inputLayouts["ParticlesInputLayout"].data(), (UINT)inputLayouts["ParticlesInputLayout"].size() };
    particlesRainPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesRainVS"]->GetBufferPointer()),
        shaders["particlesRainVS"]->GetBufferSize()
    };
    particlesRainPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesRainGS"]->GetBufferPointer()),
        shaders["particlesRainGS"]->GetBufferSize()
    };
    particlesRainPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["particlesRainPS"]->GetBufferPointer()),
        shaders["particlesRainPS"]->GetBufferSize()
    };
    particlesRainPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    particlesRainPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    particlesRainPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    particlesRainPsoDesc.RasterizerState.DepthClipEnable = TRUE;
    particlesRainPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    particlesRainPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    particlesRainPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    particlesRainPsoDesc.SampleMask = UINT_MAX;
    particlesRainPsoDesc.NumRenderTargets = 1;
    particlesRainPsoDesc.RTVFormats[0] = backBufferFormat;
    particlesRainPsoDesc.SampleDesc.Count = 1;
    particlesRainPsoDesc.SampleDesc.Quality = 0;
    particlesRainPsoDesc.DSVFormat = depthStencilFormat;
    particlesRainPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesRainPsoDesc, IID_PPV_ARGS(&mPSOs["particlesRain"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC particlesRainComputePsoDesc;
    ZeroMemory(&particlesRainComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
    particlesRainComputePsoDesc.pRootSignature = rootSignatures["mRootSignatureParticlesCompute"].Get();
    particlesRainComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(shaders["particlesRainCS"]->GetBufferPointer()),
        shaders["particlesRainCS"]->GetBufferSize()
    };
    particlesRainComputePsoDesc.NodeMask = 0;
    particlesRainComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&particlesRainComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeParticlesRain"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC terrainPsoDesc;
    ZeroMemory(&terrainPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    terrainPsoDesc = heightMapPsoDesc;
    terrainPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    terrainPsoDesc.pRootSignature = rootSignatures["mRootSignatureTerrain"].Get();
    terrainPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainVS"]->GetBufferPointer()),
        shaders["TerrainVS"]->GetBufferSize()
    };
    terrainPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainHS"]->GetBufferPointer()),
        shaders["TerrainHS"]->GetBufferSize()
    };
    terrainPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainDS"]->GetBufferPointer()),
        shaders["TerrainDS"]->GetBufferSize()
    };
    terrainPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainPS"]->GetBufferPointer()),
        shaders["TerrainPS"]->GetBufferSize()
    };
    terrainPsoDesc.NumRenderTargets = 2;
    terrainPsoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&terrainPsoDesc, IID_PPV_ARGS(&mPSOs["terrainPSO"])));

    D3D12_COMPUTE_PIPELINE_STATE_DESC terrainPaintComputePsoDesc;
    ZeroMemory(&terrainPaintComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
    terrainPaintComputePsoDesc.pRootSignature = rootSignatures["mRootSignatureTerrainPaint"].Get();
    terrainPaintComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(shaders["TerrainPaintingCS"]->GetBufferPointer()),
        shaders["TerrainPaintingCS"]->GetBufferSize()
    };
    terrainPaintComputePsoDesc.NodeMask = 0;
    terrainPaintComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&terrainPaintComputePsoDesc, IID_PPV_ARGS(&mPSOs["terrainPaintPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC terrainWireframePsoDesc;
    ZeroMemory(&terrainWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    terrainWireframePsoDesc = terrainPsoDesc;
    terrainWireframePsoDesc.InputLayout = { inputLayouts["TerrainDebugInputLayout"].data(), (UINT)inputLayouts["TerrainDebugInputLayout"].size() };
    terrainWireframePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainDebugVS"]->GetBufferPointer()),
        shaders["TerrainDebugVS"]->GetBufferSize()
    };
    terrainWireframePsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainDebugHS"]->GetBufferPointer()),
        shaders["TerrainDebugHS"]->GetBufferSize()
    };
    terrainWireframePsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainDebugDS"]->GetBufferPointer()),
        shaders["TerrainDebugDS"]->GetBufferSize()
    };
    terrainWireframePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["TerrainDebugPS"]->GetBufferPointer()),
        shaders["TerrainDebugPS"]->GetBufferSize()
    };
    terrainWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&terrainWireframePsoDesc, IID_PPV_ARGS(&mPSOs["terrainWireframePSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC TAAPsoDesc;
    ZeroMemory(&TAAPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    TAAPsoDesc = PBRPsoDesc;
    TAAPsoDesc.pRootSignature = rootSignatures["mRootSignatureTAA"].Get();
    TAAPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["TAA_VS"]->GetBufferPointer()),
        shaders["TAA_VS"]->GetBufferSize()
    };
    TAAPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["TAA_PS"]->GetBufferPointer()),
        shaders["TAA_PS"]->GetBufferSize()
    };
    TAAPsoDesc.NumRenderTargets = 2;
    TAAPsoDesc.RTVFormats[0] = backBufferFormat;
    TAAPsoDesc.RTVFormats[1] = backBufferFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&TAAPsoDesc, IID_PPV_ARGS(&mPSOs["TAAPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC TAASecondPassPsoDesc;
    ZeroMemory(&TAASecondPassPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    TAASecondPassPsoDesc = TAAPsoDesc;
    TAASecondPassPsoDesc.InputLayout = { inputLayouts["PostProcessingInputLayout"].data(), (UINT)inputLayouts["PostProcessingInputLayout"].size() };
    TAASecondPassPsoDesc.pRootSignature = rootSignatures["mRootSignatureTAASecondPass"].Get();
    TAASecondPassPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["TAASecondPass_VS"]->GetBufferPointer()),
        shaders["TAASecondPass_VS"]->GetBufferSize()
    };
    TAASecondPassPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["TAASecondPass_PS"]->GetBufferPointer()),
        shaders["TAASecondPass_PS"]->GetBufferSize()
    };
    TAASecondPassPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    TAASecondPassPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    TAASecondPassPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    TAASecondPassPsoDesc.BlendState.IndependentBlendEnable = FALSE;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    TAASecondPassPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    TAASecondPassPsoDesc.DepthStencilState.DepthEnable = FALSE;
    TAASecondPassPsoDesc.DepthStencilState.StencilEnable = FALSE;
    TAASecondPassPsoDesc.SampleMask = UINT_MAX;
    TAASecondPassPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    TAASecondPassPsoDesc.NumRenderTargets = 1;
    TAASecondPassPsoDesc.RTVFormats[0] = backBufferFormat;
    TAASecondPassPsoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
    TAASecondPassPsoDesc.SampleDesc.Count = 1;
    TAASecondPassPsoDesc.SampleDesc.Quality = 0;
    TAASecondPassPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&TAASecondPassPsoDesc, IID_PPV_ARGS(&mPSOs["TAASecondPassPSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC AtmospherePsoDesc;
    ZeroMemory(&AtmospherePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    AtmospherePsoDesc.InputLayout = { nullptr, 0 };
    AtmospherePsoDesc.pRootSignature = rootSignatures["mRootSignatureAtmosphere"].Get();
    AtmospherePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["AtmosphereVS"]->GetBufferPointer()),
        shaders["AtmosphereVS"]->GetBufferSize()
    };
    AtmospherePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["AtmospherePS"]->GetBufferPointer()),
        shaders["AtmospherePS"]->GetBufferSize()
    };
    AtmospherePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    AtmospherePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    AtmospherePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    AtmospherePsoDesc.BlendState.IndependentBlendEnable = FALSE;
    AtmospherePsoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    AtmospherePsoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
    AtmospherePsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    AtmospherePsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    AtmospherePsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    AtmospherePsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    AtmospherePsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    AtmospherePsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    AtmospherePsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    AtmospherePsoDesc.DepthStencilState.DepthEnable = FALSE;
    AtmospherePsoDesc.DepthStencilState.StencilEnable = FALSE;
    AtmospherePsoDesc.SampleMask = UINT_MAX;
    AtmospherePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    AtmospherePsoDesc.NumRenderTargets = 1;
    AtmospherePsoDesc.RTVFormats[0] = backBufferFormat;
    AtmospherePsoDesc.SampleDesc.Count = 1;
    AtmospherePsoDesc.SampleDesc.Quality = 0;
    AtmospherePsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&AtmospherePsoDesc, IID_PPV_ARGS(&mPSOs["AtmospherePSO"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC cubeMarchingPsoDesc;
    ZeroMemory(&cubeMarchingPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    cubeMarchingPsoDesc.InputLayout = { inputLayouts["CubeMarchingInputLayout"].data(), (UINT)inputLayouts["CubeMarchingInputLayout"].size() };
    cubeMarchingPsoDesc.pRootSignature = rootSignatures["mRootSignatureCubeMarching"].Get();
    cubeMarchingPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(shaders["CubeMarchingVS"]->GetBufferPointer()),
        shaders["CubeMarchingVS"]->GetBufferSize()
    };
    cubeMarchingPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(shaders["CubeMarchingPS"]->GetBufferPointer()),
        shaders["CubeMarchingPS"]->GetBufferSize()
    };
    cubeMarchingPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    cubeMarchingPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    cubeMarchingPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    cubeMarchingPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    cubeMarchingPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    cubeMarchingPsoDesc.SampleMask = UINT_MAX;
    cubeMarchingPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    cubeMarchingPsoDesc.NumRenderTargets = 1;
    cubeMarchingPsoDesc.RTVFormats[0] = backBufferFormat;
    cubeMarchingPsoDesc.SampleDesc.Count = 1;
    cubeMarchingPsoDesc.SampleDesc.Quality = 0;
    cubeMarchingPsoDesc.DSVFormat = depthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&cubeMarchingPsoDesc, IID_PPV_ARGS(&mPSOs["CubeMarchingPSO"])));
}

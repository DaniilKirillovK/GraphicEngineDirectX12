#include "RootSignatureManager.h"

void RootSignatureManager::CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc, std::string rootSigName,
    Microsoft::WRL::ComPtr<ID3D12Device> device, std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>>& rootSignatures)
{
    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&rootSignatures[rootSigName])));
}

void RootSignatureManager::BuildRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device,
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>>& rootSignatures)
{
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableSpace1;
    texTableSpace1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableDefaultForward;
    texTableDefaultForward.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableDefaultForwardSpace1;
    texTableDefaultForwardSpace1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableParticles;
    texTableParticles.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableParticlesSpace1;
    texTableParticlesSpace1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTablePostProcessing;
    texTablePostProcessing.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2PostProcessing;
    texTable2PostProcessing.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
    CD3DX12_DESCRIPTOR_RANGE texTableNoiseCompute;
    texTableNoiseCompute.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableRT;
    texTableRT.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableDefferedPointSpot;
    texTableDefferedPointSpot.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableShadowPass;
    texTableShadowPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableShadowForward0;
    texTableShadowForward0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableShadowForward1;
    texTableShadowForward1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
    CD3DX12_DESCRIPTOR_RANGE texTableShadowForward2;
    texTableShadowForward2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 2);

    CD3DX12_DESCRIPTOR_RANGE texTableShadowParticlesForward0;
    texTableShadowParticlesForward0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableShadowParticlesForward1;
    texTableShadowParticlesForward1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableSkyBox;
    texTableSkyBox.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTablePBR;
    texTablePBR.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTablePBR2;
    texTablePBR2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableRMDemo;
    texTableRMDemo.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableRMDemo2;
    texTableRMDemo2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 1);
    CD3DX12_DESCRIPTOR_RANGE texTableRMDemo3;
    texTableRMDemo3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);

    CD3DX12_DESCRIPTOR_RANGE texTableHeightMap;
    texTableHeightMap.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableMoreLight;
    texTableMoreLight.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2MoreLight;
    texTable2MoreLight.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableScene13;
    texTableScene13.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2Scene13;
    texTable2Scene13.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableParticlesRain0;
    texTableParticlesRain0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTableParticlesRain1;
    texTableParticlesRain1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    CD3DX12_DESCRIPTOR_RANGE texTableTerrain;
    texTableTerrain.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableTAA;
    texTableTAA.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable1TAASecondPass;
    texTable1TAASecondPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2TAASecondPass;
    texTable2TAASecondPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
    CD3DX12_DESCRIPTOR_RANGE texTable3TAASecondPass;
    texTable3TAASecondPass.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);

    CD3DX12_DESCRIPTOR_RANGE texTableAtmosphere;
    texTableAtmosphere.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_DESCRIPTOR_RANGE texTableCubeMarching;
    texTableCubeMarching.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[5];
    CD3DX12_ROOT_PARAMETER slotRootParameter2[2];
    CD3DX12_ROOT_PARAMETER slotRootParameterMoreSamplers[6];

    CD3DX12_ROOT_PARAMETER slotRootParameterDefaultForward[6];
    CD3DX12_ROOT_PARAMETER slotRootParameterDefaultForwardFrustumCulling[6];

    CD3DX12_ROOT_PARAMETER slotRootParameterDebug[2];

    CD3DX12_ROOT_PARAMETER slotRootParameterBillboard[4];

    CD3DX12_ROOT_PARAMETER slotRootParameterParticlesCompute[3];
    CD3DX12_ROOT_PARAMETER slotRootParameterParticlesRender[4];

    CD3DX12_ROOT_PARAMETER slotRootParameterPostProcessing[3];
    CD3DX12_ROOT_PARAMETER slotRootParameterComputeNoise[2];

    CD3DX12_ROOT_PARAMETER slotRootParameterTessellation[6];

    CD3DX12_ROOT_PARAMETER slotRootParameterRT[4];
    CD3DX12_ROOT_PARAMETER slotRootParameterDebugLayer[4];
    CD3DX12_ROOT_PARAMETER slotRootParameterDefferedPointSpot[3];
    CD3DX12_ROOT_PARAMETER slotRootParameterDefferedMoreLight[4];

    CD3DX12_ROOT_PARAMETER slotRootParameterShadowsPass[4];
    CD3DX12_ROOT_PARAMETER slotRootParameterShadowsForward[6];
    CD3DX12_ROOT_PARAMETER slotRootParameterShadowsParticlesForward[5];

    CD3DX12_ROOT_PARAMETER slotRootParameterSkybox[3];

    CD3DX12_ROOT_PARAMETER slotRootParameterPBR[5];
    CD3DX12_ROOT_PARAMETER slotRootParameterRMDemo[6];

    CD3DX12_ROOT_PARAMETER slotRootParameterParticlesArgsCompute[1];
    CD3DX12_ROOT_PARAMETER slotRootParameterParticlesGPU[5];

    CD3DX12_ROOT_PARAMETER slotRootParameterHeightMap[5];

    CD3DX12_ROOT_PARAMETER slotRootParameterObjectsScene13[5];
    CD3DX12_ROOT_PARAMETER slotRootParameterOctreeScene13[2];

    CD3DX12_ROOT_PARAMETER slotRootParameterParticlesRain[4];

    CD3DX12_ROOT_PARAMETER slotRootParameterTerrain[5];

    CD3DX12_ROOT_PARAMETER slotRootParameterTAA[4];
    CD3DX12_ROOT_PARAMETER slotRootParameterTAASecondPass[4];

    CD3DX12_ROOT_PARAMETER slotRootParameterAtmosphere[2];

    CD3DX12_ROOT_PARAMETER slotRootParameterCubeMarching[3];

    // Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsDescriptorTable(1, &texTable);
    slotRootParameter[1].InitAsDescriptorTable(1, &texTableSpace1);
    slotRootParameter[2].InitAsConstantBufferView(0);
    slotRootParameter[3].InitAsConstantBufferView(1);
    slotRootParameter[4].InitAsConstantBufferView(2);

    slotRootParameterMoreSamplers[0].InitAsDescriptorTable(1, &texTable);
    slotRootParameterMoreSamplers[1].InitAsDescriptorTable(1, &texTableSpace1);
    slotRootParameterMoreSamplers[2].InitAsConstantBufferView(0);
    slotRootParameterMoreSamplers[3].InitAsConstantBufferView(1);
    slotRootParameterMoreSamplers[4].InitAsConstantBufferView(2);
    slotRootParameterMoreSamplers[5].InitAsConstantBufferView(3);

    slotRootParameterDefaultForward[0].InitAsDescriptorTable(1, &texTableDefaultForward);
    slotRootParameterDefaultForward[1].InitAsDescriptorTable(1, &texTableDefaultForwardSpace1);
    slotRootParameterDefaultForward[2].InitAsConstantBufferView(0);
    slotRootParameterDefaultForward[3].InitAsConstantBufferView(1);
    slotRootParameterDefaultForward[4].InitAsConstantBufferView(2);
    slotRootParameterDefaultForward[5].InitAsConstantBufferView(3);

    slotRootParameterDefaultForwardFrustumCulling[0].InitAsDescriptorTable(1, &texTableDefaultForward);
    slotRootParameterDefaultForwardFrustumCulling[1].InitAsShaderResourceView(0, 1);
    slotRootParameterDefaultForwardFrustumCulling[2].InitAsConstantBufferView(0);
    slotRootParameterDefaultForwardFrustumCulling[3].InitAsConstantBufferView(1);
    slotRootParameterDefaultForwardFrustumCulling[4].InitAsConstantBufferView(2);
    slotRootParameterDefaultForwardFrustumCulling[5].InitAsConstantBufferView(3);

    slotRootParameter2[0].InitAsDescriptorTable(1, &texTable2);
    slotRootParameter2[1].InitAsConstantBufferView(0);

    slotRootParameterDebug[0].InitAsConstantBufferView(0);
    slotRootParameterDebug[1].InitAsConstantBufferView(1);

    slotRootParameterBillboard[0].InitAsDescriptorTable(1, &texTableSpace1);
    slotRootParameterBillboard[1].InitAsConstantBufferView(0);
    slotRootParameterBillboard[2].InitAsConstantBufferView(1);
    slotRootParameterBillboard[3].InitAsConstantBufferView(2);

    slotRootParameterParticlesCompute[0].InitAsUnorderedAccessView(0);
    slotRootParameterParticlesCompute[1].InitAsShaderResourceView(0);
    slotRootParameterParticlesCompute[2].InitAsConstantBufferView(0);

    slotRootParameterParticlesRender[0].InitAsDescriptorTable(1, &texTableParticles);
    slotRootParameterParticlesRender[1].InitAsDescriptorTable(1, &texTableParticlesSpace1);
    slotRootParameterParticlesRender[2].InitAsConstantBufferView(0);
    slotRootParameterParticlesRender[3].InitAsConstantBufferView(1);

    slotRootParameterPostProcessing[0].InitAsDescriptorTable(1, &texTablePostProcessing);
    slotRootParameterPostProcessing[1].InitAsConstantBufferView(0);
    slotRootParameterPostProcessing[2].InitAsDescriptorTable(1, &texTable2PostProcessing);

    slotRootParameterComputeNoise[0].InitAsDescriptorTable(1, &texTableNoiseCompute);
    slotRootParameterComputeNoise[1].InitAsConstantBufferView(0);

    slotRootParameterTessellation[0].InitAsDescriptorTable(1, &texTable);
    slotRootParameterTessellation[1].InitAsDescriptorTable(1, &texTableSpace1);
    slotRootParameterTessellation[2].InitAsConstantBufferView(0);
    slotRootParameterTessellation[3].InitAsConstantBufferView(1);
    slotRootParameterTessellation[4].InitAsConstantBufferView(2);
    slotRootParameterTessellation[5].InitAsConstantBufferView(3);

    slotRootParameterRT[0].InitAsDescriptorTable(1, &texTableRT);
    slotRootParameterRT[1].InitAsConstantBufferView(0);
    slotRootParameterRT[2].InitAsConstantBufferView(1);
    slotRootParameterRT[3].InitAsConstantBufferView(2);

    slotRootParameterDebugLayer[0].InitAsConstantBufferView(0);
    slotRootParameterDebugLayer[1].InitAsConstantBufferView(1);

    slotRootParameterDefferedPointSpot[0].InitAsDescriptorTable(1, &texTableDefferedPointSpot);
    slotRootParameterDefferedPointSpot[1].InitAsConstantBufferView(0);
    slotRootParameterDefferedPointSpot[2].InitAsConstantBufferView(1);

    slotRootParameterDefferedMoreLight[0].InitAsDescriptorTable(1, &texTableMoreLight);
    slotRootParameterDefferedMoreLight[1].InitAsConstantBufferView(0);
    slotRootParameterDefferedMoreLight[2].InitAsConstantBufferView(1);
    slotRootParameterDefferedMoreLight[3].InitAsDescriptorTable(1, &texTable2MoreLight);

    slotRootParameterShadowsPass[0].InitAsDescriptorTable(1, &texTableShadowPass);
    slotRootParameterShadowsPass[1].InitAsConstantBufferView(0);
    slotRootParameterShadowsPass[2].InitAsConstantBufferView(1);
    slotRootParameterShadowsPass[3].InitAsConstantBufferView(2);

    slotRootParameterShadowsForward[0].InitAsDescriptorTable(1, &texTableShadowForward0);
    slotRootParameterShadowsForward[1].InitAsDescriptorTable(1, &texTableShadowForward1);
    slotRootParameterShadowsForward[2].InitAsConstantBufferView(0);
    slotRootParameterShadowsForward[3].InitAsConstantBufferView(1);
    slotRootParameterShadowsForward[4].InitAsConstantBufferView(2);
    slotRootParameterShadowsForward[5].InitAsDescriptorTable(1, &texTableShadowForward2);

    slotRootParameterShadowsParticlesForward[0].InitAsDescriptorTable(1, &texTableShadowParticlesForward0);
    slotRootParameterShadowsParticlesForward[1].InitAsDescriptorTable(1, &texTableShadowParticlesForward1);
    slotRootParameterShadowsParticlesForward[2].InitAsConstantBufferView(0);
    slotRootParameterShadowsParticlesForward[3].InitAsConstantBufferView(1);
    slotRootParameterShadowsParticlesForward[4].InitAsConstantBufferView(2);

    slotRootParameterSkybox[0].InitAsDescriptorTable(1, &texTableSkyBox);
    slotRootParameterSkybox[1].InitAsConstantBufferView(0);
    slotRootParameterSkybox[2].InitAsConstantBufferView(1);

    slotRootParameterPBR[0].InitAsDescriptorTable(1, &texTablePBR);
    slotRootParameterPBR[1].InitAsConstantBufferView(0);
    slotRootParameterPBR[2].InitAsConstantBufferView(1);
    slotRootParameterPBR[3].InitAsConstantBufferView(2);
    slotRootParameterPBR[4].InitAsDescriptorTable(1, &texTablePBR2);

    slotRootParameterPBR[0].InitAsDescriptorTable(1, &texTablePBR);
    slotRootParameterPBR[1].InitAsConstantBufferView(0);
    slotRootParameterPBR[2].InitAsConstantBufferView(1);
    slotRootParameterPBR[3].InitAsConstantBufferView(2);
    slotRootParameterPBR[4].InitAsDescriptorTable(1, &texTablePBR2);

    slotRootParameterRMDemo[0].InitAsDescriptorTable(1, &texTableRMDemo);
    slotRootParameterRMDemo[1].InitAsConstantBufferView(0);
    slotRootParameterRMDemo[2].InitAsConstantBufferView(1);
    slotRootParameterRMDemo[3].InitAsConstantBufferView(2);
    slotRootParameterRMDemo[4].InitAsDescriptorTable(1, &texTableRMDemo2);
    slotRootParameterRMDemo[5].InitAsDescriptorTable(1, &texTableRMDemo3);

    slotRootParameterParticlesGPU[0].InitAsShaderResourceView(0, 0);
    slotRootParameterParticlesGPU[1].InitAsShaderResourceView(0, 1);
    slotRootParameterParticlesGPU[2].InitAsConstantBufferView(0);
    slotRootParameterParticlesGPU[3].InitAsConstantBufferView(1);
    slotRootParameterParticlesGPU[4].InitAsConstantBufferView(2);

    slotRootParameterParticlesArgsCompute[0].InitAsUnorderedAccessView(0);

    slotRootParameterHeightMap[0].InitAsDescriptorTable(1, &texTableHeightMap);
    slotRootParameterHeightMap[1].InitAsConstantBufferView(0);
    slotRootParameterHeightMap[2].InitAsConstantBufferView(1);
    slotRootParameterHeightMap[3].InitAsConstantBufferView(2);
    slotRootParameterHeightMap[4].InitAsConstantBufferView(3);

    slotRootParameterObjectsScene13[0].InitAsDescriptorTable(1, &texTableScene13);
    slotRootParameterObjectsScene13[1].InitAsConstantBufferView(0);
    slotRootParameterObjectsScene13[2].InitAsConstantBufferView(1);
    slotRootParameterObjectsScene13[3].InitAsConstantBufferView(2);
    slotRootParameterObjectsScene13[4].InitAsShaderResourceView(0, 1);

    slotRootParameterOctreeScene13[0].InitAsConstantBufferView(0);
    slotRootParameterOctreeScene13[1].InitAsConstantBufferView(1);

    slotRootParameterParticlesRain[0].InitAsDescriptorTable(1, &texTableParticlesRain0);
    slotRootParameterParticlesRain[1].InitAsDescriptorTable(1, &texTableParticlesRain1);
    slotRootParameterParticlesRain[2].InitAsConstantBufferView(0);
    slotRootParameterParticlesRain[3].InitAsConstantBufferView(1);

    slotRootParameterTerrain[0].InitAsDescriptorTable(1, &texTableTerrain);
    slotRootParameterTerrain[1].InitAsConstantBufferView(0);
    slotRootParameterTerrain[2].InitAsConstantBufferView(1);
    slotRootParameterTerrain[3].InitAsConstantBufferView(2);
    slotRootParameterTerrain[4].InitAsConstantBufferView(3);

    slotRootParameterTAA[0].InitAsDescriptorTable(1, &texTableTAA);
    slotRootParameterTAA[1].InitAsConstantBufferView(0);
    slotRootParameterTAA[2].InitAsConstantBufferView(1);
    slotRootParameterTAA[3].InitAsConstantBufferView(2);

    slotRootParameterTAASecondPass[0].InitAsDescriptorTable(1, &texTable1TAASecondPass);
    slotRootParameterTAASecondPass[1].InitAsDescriptorTable(1, &texTable2TAASecondPass);
    slotRootParameterTAASecondPass[2].InitAsConstantBufferView(0);
    slotRootParameterTAASecondPass[3].InitAsDescriptorTable(1, &texTable3TAASecondPass);

    slotRootParameterAtmosphere[0].InitAsConstantBufferView(0);
    slotRootParameterAtmosphere[1].InitAsDescriptorTable(1, &texTableAtmosphere);

    slotRootParameterCubeMarching[0].InitAsConstantBufferView(0);
    slotRootParameterCubeMarching[1].InitAsConstantBufferView(1);
    slotRootParameterCubeMarching[2].InitAsDescriptorTable(1, &texTableCubeMarching);

    auto staticSamplers = GetStaticSamplers();
    auto moreSamplers = GetMoreStaticSamplers();
    auto lodSamplers = GetLODStaticSamplers();
    auto shadowSamplers = GetLODStaticSamplersShadow();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc2(2, slotRootParameter2,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescMoreSamplers(6, slotRootParameterMoreSamplers,
        (UINT)moreSamplers.size(), moreSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescDefaultForward(6, slotRootParameterDefaultForward,
        (UINT)lodSamplers.size(), lodSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescDefaultForwardFrustumCulling(6, slotRootParameterDefaultForwardFrustumCulling,
        (UINT)lodSamplers.size(), lodSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescDebug(2, slotRootParameterDebug,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescBillboard(4, slotRootParameterBillboard,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescParticlesRender(4, slotRootParameterParticlesRender,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescParticlesCompute(3, slotRootParameterParticlesCompute,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescPostProcessing(3, slotRootParameterPostProcessing,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescComputeNoise(2, slotRootParameterComputeNoise,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescTess(6, slotRootParameterTessellation,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescRT(4, slotRootParameterRT,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescDebugLayer(2, slotRootParameterDebugLayer,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescDefferedPointSpot(3, slotRootParameterDefferedPointSpot,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescShadowsPass(4, slotRootParameterShadowsPass,
        (UINT)shadowSamplers.size(), shadowSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescShadowsForward(6, slotRootParameterShadowsForward,
        (UINT)shadowSamplers.size(), shadowSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescShadowsParticlesForward(5, slotRootParameterShadowsParticlesForward,
        (UINT)shadowSamplers.size(), shadowSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescSkybox(3, slotRootParameterSkybox,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescPBR(5, slotRootParameterPBR,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescRMDemo(6, slotRootParameterRMDemo,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescParticlesArgsCompute(1, slotRootParameterParticlesArgsCompute,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescParticlesGPU(5, slotRootParameterParticlesGPU,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescHeightMap(5, slotRootParameterHeightMap,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescMoreLight(4, slotRootParameterDefferedMoreLight,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescObjectsScene13(5, slotRootParameterObjectsScene13,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescOctreeScene13(2, slotRootParameterOctreeScene13,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescParticlesRain(4, slotRootParameterParticlesRain,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescTerrain(5, slotRootParameterTerrain,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescTAA(4, slotRootParameterTAA,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescTAASecondPass(4, slotRootParameterTAASecondPass,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescAtmosphere(2, slotRootParameterAtmosphere,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDescCubeMarching(3, slotRootParameterCubeMarching,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    CreateRootSignature(rootSigDesc, "mRootSignature", device, rootSignatures);
    CreateRootSignature(rootSigDesc2, "mRootSignatureLight", device, rootSignatures);
    CreateRootSignature(rootSigDescMoreSamplers, "mRootSignatureMoreSamplers", device, rootSignatures);
    CreateRootSignature(rootSigDescDefaultForward, "mRootSignatureDefaultForward", device, rootSignatures);
    CreateRootSignature(rootSigDescDefaultForwardFrustumCulling, "mRootSignatureDefaultForwardFrustumCulling", device, rootSignatures);
    CreateRootSignature(rootSigDescDebug, "mRootSignatureDebug", device, rootSignatures);
    CreateRootSignature(rootSigDescBillboard, "mRootSignatureBillboard", device, rootSignatures);
    CreateRootSignature(rootSigDescDebug, "mRootSignatureDebug", device, rootSignatures);
    CreateRootSignature(rootSigDescParticlesRender, "mRootSignatureParticlesRender", device, rootSignatures);
    CreateRootSignature(rootSigDescParticlesCompute, "mRootSignatureParticlesCompute", device, rootSignatures);
    CreateRootSignature(rootSigDescPostProcessing, "mRootSignaturePostProcessing", device, rootSignatures);
    CreateRootSignature(rootSigDescComputeNoise, "mRootSignatureComputeNoise", device, rootSignatures);
    CreateRootSignature(rootSigDescTess, "mRootSignatureTess", device, rootSignatures);
    CreateRootSignature(rootSigDescRT, "mRootSignatureRT", device, rootSignatures);
    CreateRootSignature(rootSigDescShadowsPass, "mRootSignatureShadows", device, rootSignatures);
    CreateRootSignature(rootSigDescShadowsForward, "mRootSignatureShadowsForward", device, rootSignatures);
    CreateRootSignature(rootSigDescShadowsParticlesForward, "mRootSignatureShadowsParticlesForward", device, rootSignatures);
    CreateRootSignature(rootSigDescDebugLayer, "mRootSignatureDebugGeometry", device, rootSignatures);
    CreateRootSignature(rootSigDescDefferedPointSpot, "mRootSignatureDefferedPointSpotDirectional", device, rootSignatures);
    CreateRootSignature(rootSigDescSkybox, "mRootSignatureSkyBox", device, rootSignatures);
    CreateRootSignature(rootSigDescPBR, "mRootSignaturePBR", device, rootSignatures);
    CreateRootSignature(rootSigDescRMDemo, "mRootSignatureRMDemo", device, rootSignatures);
    CreateRootSignature(rootSigDescParticlesArgsCompute, "mRootSignatureParticlesArgsCompute", device, rootSignatures);
    CreateRootSignature(rootSigDescParticlesGPU, "mRootSignatureParticlesGPU", device, rootSignatures);
    CreateRootSignature(rootSigDescHeightMap, "mRootSignatureHeightMap", device, rootSignatures);
    CreateRootSignature(rootSigDescMoreLight, "mRootSignatureMoreLight", device, rootSignatures);
    CreateRootSignature(rootSigDescObjectsScene13, "mRootSignatureObjectsScene13", device, rootSignatures);
    CreateRootSignature(rootSigDescOctreeScene13, "mRootSignatureOctreeScene13", device, rootSignatures);
    CreateRootSignature(rootSigDescParticlesRain, "mRootSignatureParticlesRain", device, rootSignatures);
    CreateRootSignature(rootSigDescTerrain, "mRootSignatureTerrain", device, rootSignatures);
    CreateRootSignature(rootSigDescTAA, "mRootSignatureTAA", device, rootSignatures);
    CreateRootSignature(rootSigDescTAASecondPass, "mRootSignatureTAASecondPass", device, rootSignatures);
    CreateRootSignature(rootSigDescAtmosphere, "mRootSignatureAtmosphere", device, rootSignatures);
    CreateRootSignature(rootSigDescCubeMarching, "mRootSignatureCubeMarching", device, rootSignatures);
}

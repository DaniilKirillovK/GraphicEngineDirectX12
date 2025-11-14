#include "InputLayoutShaderManager.h"

void InputLayoutShaderManager::BuildShadersAndInputLayout(std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>>& inputLayouts,
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>& shaders)
{
    shaders["forwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefaultForward.hlsl", nullptr, "VS", "vs_5_1");
    shaders["forwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefaultForward.hlsl", nullptr, "PS", "ps_5_1");

    shaders["tessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "VS", "vs_5_1");
    shaders["tessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "HS", "hs_5_1");
    shaders["tessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "DS", "ds_5_1");
    shaders["tessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PS", "ps_5_1");
    shaders["PSPixel"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PSPixel", "ps_5_1");

    shaders["DeferredVSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "VSMain", "vs_5_1");
    shaders["DeferredPSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "PSMain", "ps_5_1");

    shaders["DefferedVSLightingMoreLight"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingMoreLight.hlsl", nullptr, "VSMain", "vs_5_1");
    shaders["DefferedPSLightingMoreLight"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingMoreLight.hlsl", nullptr, "PSMain", "ps_5_1");

    shaders["debugVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "VS", "vs_5_0");
    shaders["debugPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "PS", "ps_5_0");

    shaders["debugGeometryVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugGeometry.hlsl", nullptr, "VS", "vs_5_0");
    shaders["debugGeometryPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugGeometry.hlsl", nullptr, "PS", "ps_5_0");

    shaders["billboardSpriteVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "VS", "vs_5_1");
    shaders["billboardSpriteGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "GS", "gs_5_1");
    shaders["billboardSpritePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "PS", "ps_5_1");

    shaders["particlesVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "VS", "vs_5_1");
    shaders["particlesGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "GS", "gs_5_1");
    shaders["particlesPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "PS", "ps_5_1");

    shaders["particlesForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "VS", "vs_5_1");
    shaders["particlesForwardGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "GS", "gs_5_1");
    shaders["particlesForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "PS", "ps_5_1");

    shaders["particlesRainVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "VS", "vs_5_1");
    shaders["particlesRainGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "GS", "gs_5_1");
    shaders["particlesRainPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "PS", "ps_5_1");

    shaders["particlesForwardVertexLightingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "VS", "vs_5_1");
    shaders["particlesForwardVertexLightingGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "GS", "gs_5_1");
    shaders["particlesForwardVertexLightingPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "PS", "ps_5_1");

    shaders["particlesCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeParticles.hlsl", nullptr, "CS_UpdateParticles", "cs_5_1");
    shaders["particlesRainCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeParticlesRain.hlsl", nullptr, "CS_UpdateParticles", "cs_5_1");

    shaders["particlesArgsCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeArgsParticles.hlsl", nullptr, "CS_UpdateArgs", "cs_5_1");

    shaders["PostProcessingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "VS", "vs_5_1");
    shaders["PostProcessingPS_GC"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSGammaCorrection", "ps_5_1");
    shaders["PostProcessingPS_GB"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSGaussianBlur", "ps_5_1");
    shaders["PostProcessingPS_CA"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSChromaticAberration", "ps_5_1");
    shaders["PostProcessingPS_Vig"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSVignette", "ps_5_1");
    shaders["PostProcessingPS_Noise"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSNoise", "ps_5_1");
    shaders["PostProcessingPS_Default"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "DefaultPS", "ps_5_1");

    shaders["noiseCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeNoise.hlsl", nullptr, "CSMain", "cs_5_1");

    shaders["moreSamplersVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "VS", "vs_5_1");
    shaders["moreSamplersHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "HS", "hs_5_1");
    shaders["moreSamplersDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "DS", "ds_5_1");
    shaders["moreSamplersPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "PS", "ps_5_1");

    shaders["forwardRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "VS", "vs_5_1");
    shaders["forwardRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "PS", "ps_5_1");

    shaders["forwardRTMoreLight_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ForwardMoreLight.hlsl", nullptr, "VS", "vs_5_1");
    shaders["forwardRTMoreLight_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ForwardMoreLight.hlsl", nullptr, "PS", "ps_5_1");

    shaders["defferedRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometry.hlsl", nullptr, "VS", "vs_5_1");
    shaders["defferedRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometry.hlsl", nullptr, "PS", "ps_5_1");

    shaders["defferedNoPosRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometryNoPos.hlsl", nullptr, "VS", "vs_5_1");
    shaders["defferedNoPosRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometryNoPos.hlsl", nullptr, "PS", "ps_5_1");

    shaders["DefferedLightingNoPosRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingNoPos.hlsl", nullptr, "VSMain", "vs_5_1");
    shaders["DefferedLightingNoPosRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingNoPos.hlsl", nullptr, "PSMain", "ps_5_1");

    shaders["defferedPointSpotVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPoint.hlsl", nullptr, "VS", "vs_5_1");
    shaders["defferedPointSpotPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPoint.hlsl", nullptr, "PS", "ps_5_1");

    shaders["defferedPointSpotMoreLightVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPointMoreLight.hlsl", nullptr, "VS", "vs_5_1");
    shaders["defferedPointSpotMoreLightPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPointMoreLight.hlsl", nullptr, "PS", "ps_5_1");

    shaders["defferedDirectionalVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedDirectionalLight.hlsl", nullptr, "VS", "vs_5_1");
    shaders["defferedDirectionalPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedDirectionalLight.hlsl", nullptr, "PS", "ps_5_1");

    shaders["decalsTessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "VS", "vs_5_1");
    shaders["decalsTessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "HS", "hs_5_1");
    shaders["decalsTessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "DS", "ds_5_1");
    shaders["decalsTessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "PS", "ps_5_1");

    shaders["shadowVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
    shaders["shadowPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");

    shaders["shadowParticlesVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "VS", "vs_5_1");
    shaders["shadowParticlesGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "GS", "gs_5_1");
    shaders["shadowParticlesPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "PS", "ps_5_1");
    
    shaders["cascadedShadowsForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CascadedShadowsForward.hlsl", nullptr, "VS", "vs_5_1");
    shaders["cascadedShadowsForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CascadedShadowsForward.hlsl", nullptr, "PS", "ps_5_1");

    shaders["shadowsForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsForward.hlsl", nullptr, "VS", "vs_5_1");
    shaders["shadowsForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsForward.hlsl", nullptr, "PS", "ps_5_1");

    shaders["skyboxVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\SkyBox.hlsl", nullptr, "VS", "vs_5_1");
    shaders["skyboxPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\SkyBox.hlsl", nullptr, "PS", "ps_5_1");

    shaders["PBR_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PBR.hlsl", nullptr, "VS", "vs_5_1");
    shaders["PBR_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PBR.hlsl", nullptr, "PS", "ps_5_1");

    shaders["StandartPBR_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\StandartPBR.hlsl", nullptr, "VS", "vs_5_1");
    shaders["StandartPBR_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\StandartPBR.hlsl", nullptr, "PS", "ps_5_1");

    shaders["IBL_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\IBL.hlsl", nullptr, "VS", "vs_5_1");
    shaders["IBL_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\IBL.hlsl", nullptr, "PS", "ps_5_1");

    shaders["RMDemoVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\RoughnessMetallicDemo.hlsl", nullptr, "VS", "vs_5_1");
    shaders["RMDemoPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\RoughnessMetallicDemo.hlsl", nullptr, "PS", "ps_5_1");

    shaders["HeightMapVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "VS", "vs_5_1");
    shaders["HeightMapHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "HS", "hs_5_1");
    shaders["HeightMapDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "DS", "ds_5_1");
    shaders["HeightMapPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "PS", "ps_5_1");

    shaders["Scene13VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Objects.hlsl", nullptr, "VS", "vs_5_1");
    shaders["Scene13PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Objects.hlsl", nullptr, "PS", "ps_5_1");

    shaders["Scene13OcTreeVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Octree.hlsl", nullptr, "VS", "vs_5_1");
    shaders["Scene13OcTreePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Octree.hlsl", nullptr, "PS", "ps_5_1");

    shaders["TerrainVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "VS", "vs_5_1");
    shaders["TerrainHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "HS", "hs_5_1");
    shaders["TerrainDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "DS", "ds_5_1");
    shaders["TerrainPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "PS", "ps_5_1");

    shaders["TerrainDebugVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "VS", "vs_5_1");
    shaders["TerrainDebugHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "HS", "hs_5_1");
    shaders["TerrainDebugDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "DS", "ds_5_1");
    shaders["TerrainDebugPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "PS", "ps_5_1");

    shaders["TAA_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAA.hlsl", nullptr, "VS", "vs_5_1");
    shaders["TAA_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAA.hlsl", nullptr, "PS", "ps_5_1");

    shaders["TAASecondPass_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAASecondPass.hlsl", nullptr, "VS", "vs_5_1");
    shaders["TAASecondPass_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAASecondPass.hlsl", nullptr, "PS", "ps_5_1");

    shaders["AtmosphereVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Atmosphere.hlsl", nullptr, "VS", "vs_5_1");
    shaders["AtmospherePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Atmosphere.hlsl", nullptr, "PS", "ps_5_1");

    shaders["CubeMarchingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CubeMarching.hlsl", nullptr, "VS", "vs_5_1");
    shaders["CubeMarchingPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CubeMarching.hlsl", nullptr, "PS", "ps_5_1");

    inputLayouts["InputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    inputLayouts["InputLayoutLight"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    inputLayouts["BillboardSpriteInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    inputLayouts["ParticlesInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    inputLayouts["PostProcessingInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    inputLayouts["DebugInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    inputLayouts["TerrainDebugInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    inputLayouts["CubeMarchingInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

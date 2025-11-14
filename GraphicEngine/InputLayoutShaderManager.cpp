#include "InputLayoutShaderManager.h"

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> InputLayoutShaderManager::mShaders = std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>();
std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayoutShaderManager::mInputLayouts = std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>>();


void InputLayoutShaderManager::BuildShadersAndInputLayout()
{
    mShaders["forwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefaultForward.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["forwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefaultForward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["tessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["tessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["tessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["tessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PS", "ps_5_1");
    mShaders["PSPixel"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PSPixel", "ps_5_1");

    mShaders["DeferredVSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "VSMain", "vs_5_1");
    mShaders["DeferredPSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "PSMain", "ps_5_1");

    mShaders["DefferedVSLightingMoreLight"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingMoreLight.hlsl", nullptr, "VSMain", "vs_5_1");
    mShaders["DefferedPSLightingMoreLight"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingMoreLight.hlsl", nullptr, "PSMain", "ps_5_1");

    mShaders["debugVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "VS", "vs_5_0");
    mShaders["debugPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "PS", "ps_5_0");

    mShaders["debugGeometryVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugGeometry.hlsl", nullptr, "VS", "vs_5_0");
    mShaders["debugGeometryPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugGeometry.hlsl", nullptr, "PS", "ps_5_0");

    mShaders["billboardSpriteVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["billboardSpriteGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["billboardSpritePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["particlesGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["particlesPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["particlesForwardGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["particlesForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesRainVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["particlesRainGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["particlesRainPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesRain.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesForwardVertexLightingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["particlesForwardVertexLightingGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["particlesForwardVertexLightingPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ParticlesForwardVertexLighting.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeParticles.hlsl", nullptr, "CS_UpdateParticles", "cs_5_1");
    mShaders["particlesRainCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeParticlesRain.hlsl", nullptr, "CS_UpdateParticles", "cs_5_1");

    mShaders["particlesArgsCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeArgsParticles.hlsl", nullptr, "CS_UpdateArgs", "cs_5_1");

    mShaders["PostProcessingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["PostProcessingPS_GC"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSGammaCorrection", "ps_5_1");
    mShaders["PostProcessingPS_GB"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSGaussianBlur", "ps_5_1");
    mShaders["PostProcessingPS_CA"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSChromaticAberration", "ps_5_1");
    mShaders["PostProcessingPS_Vig"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSVignette", "ps_5_1");
    mShaders["PostProcessingPS_Noise"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "PSNoise", "ps_5_1");
    mShaders["PostProcessingPS_Default"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PostProcessing.hlsl", nullptr, "DefaultPS", "ps_5_1");

    mShaders["noiseCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeNoise.hlsl", nullptr, "CSMain", "cs_5_1");

    mShaders["moreSamplersVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["moreSamplersHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["moreSamplersDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["moreSamplersPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\MoreTextureSamples.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["forwardRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["forwardRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["forwardRTMoreLight_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ForwardMoreLight.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["forwardRTMoreLight_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ForwardMoreLight.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["defferedRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometry.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["defferedRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometry.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["defferedNoPosRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometryNoPos.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["defferedNoPosRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedGeometryNoPos.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["DefferedLightingNoPosRT_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingNoPos.hlsl", nullptr, "VSMain", "vs_5_1");
    mShaders["DefferedLightingNoPosRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedLightingNoPos.hlsl", nullptr, "PSMain", "ps_5_1");

    mShaders["defferedPointSpotVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPoint.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["defferedPointSpotPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPoint.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["defferedPointSpotMoreLightVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPointMoreLight.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["defferedPointSpotMoreLightPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedSpotAndPointMoreLight.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["defferedDirectionalVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedDirectionalLight.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["defferedDirectionalPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DefferedDirectionalLight.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["decalsTessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["decalsTessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["decalsTessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["decalsTessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["shadowVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Shadows.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["shadowPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Shadows.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["shadowParticlesVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["shadowParticlesGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["shadowParticlesPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsParticles.hlsl", nullptr, "PS", "ps_5_1");
    
    mShaders["cascadedShadowsForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CascadedShadowsForward.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["cascadedShadowsForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CascadedShadowsForward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["shadowsForwardVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsForward.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["shadowsForwardPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ShadowsForward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["skyboxVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\SkyBox.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["skyboxPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\SkyBox.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["PBR_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PBR.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["PBR_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\PBR.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["StandartPBR_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\StandartPBR.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["StandartPBR_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\StandartPBR.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["IBL_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\IBL.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["IBL_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\IBL.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["RMDemoVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\RoughnessMetallicDemo.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["RMDemoPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\RoughnessMetallicDemo.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["HeightMapVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["HeightMapHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["HeightMapDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["HeightMapPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\HeightMapTerrain.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["Scene13VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Objects.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["Scene13PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Objects.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["Scene13OcTreeVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Octree.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["Scene13OcTreePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Scene13Octree.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["TerrainVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["TerrainHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["TerrainDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["TerrainPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Terrain.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["TerrainDebugVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["TerrainDebugHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["TerrainDebugDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["TerrainDebugPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TerrainDebug.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["TAA_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAA.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["TAA_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAA.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["TAASecondPass_VS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAASecondPass.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["TAASecondPass_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\TAASecondPass.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["AtmosphereVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Atmosphere.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["AtmospherePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Atmosphere.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["CubeMarchingVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CubeMarching.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["CubeMarchingPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\CubeMarching.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayouts["InputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    mInputLayouts["InputLayoutLight"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mInputLayouts["BillboardSpriteInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mInputLayouts["ParticlesInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mInputLayouts["PostProcessingInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mInputLayouts["DebugInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mInputLayouts["TerrainDebugInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    mInputLayouts["CubeMarchingInputLayout"] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

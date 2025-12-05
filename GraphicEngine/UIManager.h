#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "D3D12Engine.h"
#include "FrameResource.h"

// Imgui Variables
extern bool opened;

extern int activeSceneID;

extern bool isDebug;
extern bool gridIsActive;

extern bool flashlightIsActive;

extern bool isNegative;

extern int tilesCountInt;
extern int selectedObjectID;
extern bool isAnimateMaterial;
extern bool isSolid;
extern bool deferredRenderDisplayInfo;
extern bool isParallaxMapping;
extern bool isDeferredRender;

extern bool fpsObjectIsActive;

extern float displacementLevel;

extern bool isPixelated;
extern int pixelationFactor;

extern int instancingLevel;

extern float Obj1Scale;

extern float Obj1posX;
extern float Obj1posY;
extern float Obj1posZ;

extern float Obj1rotX;
extern float Obj1rotY;
extern float Obj1rotZ;


extern float Obj2posX;
extern float Obj2posY;
extern float Obj2posZ;

extern float Obj2rotX;
extern float Obj2rotY;
extern float Obj2rotZ;

extern float tessFactor;

extern float col1[3];
extern float col2[3];
extern float col3[3];

extern float lightPos1[3];
extern float lightPos2[3];
extern float lightPos3[3];

extern float light1Strength;
extern float light2Strength;
extern float light3Strength;

extern float light1Distance;
extern float light2Distance;
extern float light3Distance;


extern float lightPosSpot1[3];
extern float spotLight1Direction[3];
extern float light1SpotStrength;
extern float light1SpotDistance;
extern float spotLight1Power;
extern float colSpot1[3];

extern float light2SpotStrength;
extern float light2SpotDistance;
extern float spotLight2Power;
extern float colSpot2[3];

extern bool isFrustumCullingScene3;
extern bool isDisplayingFrustumCullingInfoScene3;
extern bool isUsingInstancingScene3;
extern bool isUsingManualLODScene3;
extern int levelOfDetailsScene3;

extern bool isAnimateMaterialScene4;
extern int tilesCountScene4;
extern int filteringModeScene4;
extern int addressModeScene4;

extern float tessFactorScene5;
extern bool isSolidScene5;
extern float displacementLevelScene5;


extern int activeParticleSystemScene6;

extern bool particles1IsActive;
extern float particleStartColorScene6[3];
extern float particleEndColorScene6[3];
extern float particleStartSizeScene6;
extern float particleEndSizeScene6;

extern bool particles2IsActive;
extern float particle2StartColorScene6[3];
extern float particle2EndColorScene6[3];
extern float particle2StartSizeScene6;
extern float particle2EndSizeScene6;

extern int selectedEffectScene8;
extern bool isActiveNormalScene8;
extern bool isActiveGCScene8;
extern bool isActiveGBScene8;
extern bool isActiveCAScene8;
extern bool isActiveVigScene8;
extern bool isActiveNoiseScene8;
extern float gammaRatioScene8;
extern float textureSizeScene8;
extern DirectX::XMFLOAT2 caDistortionScene8;
extern DirectX::XMFLOAT2 caDirectionScene8;
extern bool gbIsHorizontalScene8;
extern float caIntensityScene8;
extern float caPaddingScene8;
extern DirectX::XMFLOAT2 vCenterScene8;
extern float vIntensityScene8;
extern float vSmoothnessScene8;
extern float vRoundnessScene8;
extern float nIntensityScene8;
extern float nSizeScene8;

extern int selectedRenderTechScene10;
extern bool deferredRenderDisplayInfoScene10;
extern bool isDebugLayerActiveScene10;
extern float col1Scene10[3];
extern float col2Scene10[3];
extern float col3Scene10[3];
extern float lightPos1Scene10[3];
extern float lightPos2Scene10[3];
extern float lightPos3Scene10[3];
extern float light1StrengthScene10;
extern float light2StrengthScene10;
extern float light3StrengthScene10;
extern float light1DistanceScene10;
extern float light2DistanceScene10;
extern float light3DistanceScene10;
extern float lightPosSpot1Scene10[3];
extern float spotLight1DirectionScene10[3];
extern float light1SpotStrengthScene10;
extern float light1SpotDistanceScene10;
extern float spotLight1PowerScene10;
extern float colSpot1Scene10[3];
extern bool isUsingPositionBufferScene10;

extern bool isMoreLightActiveScene10;
extern float moreLightDistanceScene10;
extern float moreLightColScene10[3];
extern float moreLightStrengthScene10;

extern bool isWireframeScene11;;
extern float tessFactorScene11;
extern bool bIsBackCullingScene11;
extern bool bIsDisplacementAdaptiveTessScene11;
extern bool bIsDistantAdaptiveTessScene11;
extern DirectX::XMFLOAT3 decalsPositionScene11[3];
extern float decalsDisplacementScaleScene11[3];
extern bool decalsIsActiveScene11[3];
extern float decalsScaleScene11[3];

extern float directionalLightPositionScene7;
extern bool isTexturedShadowsScene7;
extern int shadowTextureIDScene7;
extern bool isUsingCascadedShadowsScene7;
extern int shadowSizeIDScene7;
extern int shadowFilteringIDScene7;
extern bool isVerticalLightScene7;
extern bool isObjectsActiveScene7;

extern int lightingIDScene6;

extern int PBRShaderScene9;
extern bool isRoughnessMetallicDemoScene9;

extern bool isWireframeScene12;
extern float tessFactorScene12;
extern bool isDistantAdaptiveTessScene12;
extern float displacementScaleScene12;

extern bool isDebugOctreeScene13;
extern bool isActiveOctreeCullingInfoScene13;
extern bool isUsingOctreeCullingScene13;

extern float displacementScaleScene14;
extern bool isWireframeScene14;
extern bool bIsPaintingScene14;

extern bool isMovingObjectScene15;
extern int selectedRenderModeScene15;
extern bool bIsPausedScene15;
extern bool drawScene15;

extern float scene16SunColor[3];
extern float scene16ScaterringIntensity;
extern float scene16RayleiCoef[3];
extern float scene16MieCoef;
extern DirectX::XMFLOAT3 scene16SunPosition;



static class UIManager
{
public:
	static void RenderUI(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        std::vector<std::unique_ptr<RenderItem>>& allRitems,
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& srvHeap,
        UINT cbvSrvDescriptorSize,
        std::unordered_map<std::string, std::unique_ptr<Material>>& materials);
};


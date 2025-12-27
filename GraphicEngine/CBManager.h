#pragma once
#include "UIManager.h"
#include "OcTree.h"
#include "MarchingCubes.h"
#include "ShadowMap.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "TAAUtility.h"
#include "GeometryManager.h"

extern float tilesCount;

extern std::string timeScene2;
extern std::vector<InstanceData> instancesData;
extern std::vector<InstanceDataRMDemo> instancesDataRMDemo;
extern std::vector<InstanceDataMoreLight> instancesDataMoreLight;
extern std::vector<InstanceDataGameObject> instancesDataOcTree;
extern std::vector<DirectX::XMMATRIX> instanceDataScene3;
extern DirectX::BoundingFrustum mCamFrustum;

extern std::unique_ptr<ShadowMap> mShadowMap256;
extern std::unique_ptr<ShadowMap> mShadowMap512;
extern std::unique_ptr<ShadowMap> mShadowMap1024;
extern std::unique_ptr<ShadowMap> mShadowMap2048;
extern std::unique_ptr<ShadowMap> mShadowMapScene6;

extern float mLightNearZ;
extern float mLightFarZ;
extern DirectX::XMFLOAT3 mLightPosW;
extern DirectX::XMFLOAT4X4 mLightView;
extern DirectX::XMFLOAT4X4 mLightProj;
extern DirectX::XMFLOAT4X4 mShadowTransform;

extern DirectX::XMFLOAT4X4 mShadowTransform256;
extern DirectX::XMFLOAT4X4 mLightView256;
extern DirectX::XMFLOAT4X4 mLightProj256;

extern DirectX::XMFLOAT4X4 mShadowTransform512;
extern DirectX::XMFLOAT4X4 mLightView512;
extern DirectX::XMFLOAT4X4 mLightProj512;

extern DirectX::XMFLOAT4X4 mShadowTransform1024;
extern DirectX::XMFLOAT4X4 mLightView1024;
extern DirectX::XMFLOAT4X4 mLightProj1024;

extern DirectX::XMFLOAT4X4 mShadowTransform2048;
extern DirectX::XMFLOAT4X4 mLightView2048;
extern DirectX::XMFLOAT4X4 mLightProj2048;

extern float mLightRotationAngle;
extern DirectX::XMFLOAT3 mBaseLightDirections[3];
extern DirectX::XMFLOAT3 mRotatedLightDirections[3];

extern int LODScene3;

extern std::vector<GameObject*> gameObjects;
extern Octree* octreeScene13;

extern DirectX::XMFLOAT3 m_Scene15ObjectPostion;

extern DirectX::XMFLOAT4X4 PreviousWorld;
extern DirectX::XMFLOAT4X4 PreviousView;
extern DirectX::XMFLOAT4X4 PreviousProj;

extern MarchingCubes* m_MarchingCubes;

class CBManager
{
public:
    static void Initialize(FrameResource* currFrameResource, std::vector<std::unique_ptr<RenderItem>>& allRitems);

    static void UpdateObjectCBs(const GameTimer& gt);
    static void UpdateLightObjectCBs(const GameTimer& gt);
    static void UpdateLightObjectCBMoreLight(const GameTimer& gt);
    static void UpdateTAAObjectCBs(const GameTimer& gt);

    static void UpdateMaterialCBs(const GameTimer& gt);
    static void UpdateMainPassCB(const GameTimer& gt);
    static void UpdateMainPassCBScene10(const GameTimer& gt);
    static void UpdateMainPassCBMoreLightScene10(const GameTimer& gt);
    static void UpdateMainPassCBParticles(const GameTimer& gt);
    static void UpdateMainPassCBScene3Camera2(const GameTimer& gt);
    static void UpdateMainPassCBScene13Camera2(const GameTimer& gt);
    static void UpdateMainPassCBShadows(const GameTimer& gt);
    static void UpdateMainPassCBShadowsCascaded(const GameTimer& gt);
    static void UpdateShadowPassCB(const GameTimer& gt);
    static void UpdateShadowPassCBCascaded(const GameTimer& gt);
    static void UpdateShadowPassCBParticles(const GameTimer& gt);
    static void UpdateShadowTransform(const GameTimer& gt);
    static void UpdateMainPassCBScene15(const GameTimer& gt);

    static void UpdateShadowTransformCascaded(Camera camera, int MapID, DirectX::XMFLOAT3 center,
        float l, float r, float b, float t, float n, float f);

    static void UpdateParticleEmitterCB(const GameTimer& gt);
    static void UpdateParticleEmitter2CB(const GameTimer& gt);
    static void UpdateParticleEmitter3CB(const GameTimer& gt);
    static void UpdateParticleEmitter4CB(const GameTimer& gt);
    static void UpdatePostProcessingCB(const GameTimer& gt);
    static void UpdateNoiseCB(const GameTimer& gt);
    static void UpdateSamplersCB(const GameTimer& gt);
    static void UpdateLODCB(const GameTimer& gt);
    static void UpdateTessCB();
    static void UpdateHeightMapCB();
    static void UpdateTerrainCB();
    static void UpdateScene15ObjectPosition(const GameTimer& gt);
    static void UpdateAtmosphereCB();
    static void UpdatePaintClickCB();
    static void UpdateMeshRenderCB(const GameTimer& gt);

    static std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    static FrameResource* mCurrFrameResource;
    static int mCurrFrameResourceIndex;

    static Camera* mCamera;
    static Camera* mCamera2Scene3;
    static Camera* mCamera2Scene13;

    static Camera* mCameraFrustum3;
    static Camera* mCameraFrustum2;
    static Camera* mCameraFrustum1;
    static Camera* mCameraFrustum0;

    static Camera* mCameraShadowMap256;
    static Camera* mCameraShadowMap512;
    static Camera* mCameraShadowMap1024;
    static Camera* mCameraShadowMap2048;

    static ParticleSystem* mParticleSystem;
    static ParticleSystem* mParticleSystem2;
    static ParticleSystem* mParticleSystemSmoke;
    static ParticleSystem* mParticleSystemRain;

    static int mClientWidth;
    static int mClientHeight;
};


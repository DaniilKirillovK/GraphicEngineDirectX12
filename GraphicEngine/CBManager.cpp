#include "CBManager.h"
#include "UIManager.h"
#include "CommonData.h"
#include "MeshPipeline.h"

int CBManager::mClientWidth = 1280;
int CBManager::mClientHeight = 800;
Camera* CBManager::mCamera = new Camera();
Camera* CBManager::mCamera2Scene3 = new Camera();
Camera* CBManager::mCamera2Scene13 = new Camera();

Camera* CBManager::mCameraFrustum3 = new Camera();
Camera* CBManager::mCameraFrustum2 = new Camera();
Camera* CBManager::mCameraFrustum1 = new Camera();
Camera* CBManager::mCameraFrustum0 = new Camera();

Camera* CBManager::mCameraShadowMap256 = new Camera();
Camera* CBManager::mCameraShadowMap512 = new Camera();
Camera* CBManager::mCameraShadowMap1024 = new Camera();
Camera* CBManager::mCameraShadowMap2048 = new Camera();

ParticleSystem* CBManager::mParticleSystem = new ParticleSystem();
ParticleSystem* CBManager::mParticleSystem2 = new ParticleSystem();
ParticleSystem* CBManager::mParticleSystemSmoke = new ParticleSystem();
ParticleSystem* CBManager::mParticleSystemRain = new ParticleSystem();

int CBManager::mCurrFrameResourceIndex = 0;
FrameResource* CBManager::mCurrFrameResource;
std::vector<std::unique_ptr<FrameResource>> CBManager::mFrameResources = std::vector<std::unique_ptr<FrameResource>>();




float tilesCount = 1.0f;

std::string timeScene2;
std::vector<InstanceData> instancesData;
std::vector<InstanceDataRMDemo> instancesDataRMDemo;
std::vector<InstanceDataMoreLight> instancesDataMoreLight;
std::vector<InstanceDataGameObject> instancesDataOcTree;
std::vector<DirectX::XMMATRIX> instanceDataScene3;
DirectX::BoundingFrustum mCamFrustum;

std::unique_ptr<ShadowMap> mShadowMap256;
std::unique_ptr<ShadowMap> mShadowMap512;
std::unique_ptr<ShadowMap> mShadowMap1024;
std::unique_ptr<ShadowMap> mShadowMap2048;
std::unique_ptr<ShadowMap> mShadowMapScene6;

float mLightNearZ = 0.0f;
float mLightFarZ = 0.0f;
DirectX::XMFLOAT3 mLightPosW;
DirectX::XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

DirectX::XMFLOAT4X4 mShadowTransform256 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightView256 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightProj256 = MathHelper::Identity4x4();

DirectX::XMFLOAT4X4 mShadowTransform512 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightView512 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightProj512 = MathHelper::Identity4x4();

DirectX::XMFLOAT4X4 mShadowTransform1024 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightView1024 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightProj1024 = MathHelper::Identity4x4();

DirectX::XMFLOAT4X4 mShadowTransform2048 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightView2048 = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 mLightProj2048 = MathHelper::Identity4x4();

float mLightRotationAngle = 0.0f;
DirectX::XMFLOAT3 mBaseLightDirections[3] =
{
    DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
    DirectX::XMFLOAT3(0.0f, -0.9999f, -0.0141f),
    DirectX::XMFLOAT3(0.0f, -0.707f, -0.707f)
};
DirectX::XMFLOAT3 mRotatedLightDirections[3];

int LODScene3 = 3;

std::vector<GameObject*> gameObjects;
Octree* octreeScene13;

DirectX::XMFLOAT3 m_Scene15ObjectPostion = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

DirectX::XMFLOAT4X4 PreviousWorld = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 PreviousView = MathHelper::Identity4x4();
DirectX::XMFLOAT4X4 PreviousProj = MathHelper::Identity4x4();

MarchingCubes* m_MarchingCubes;


void CBManager::Initialize(FrameResource* currFrameResource, std::vector<std::unique_ptr<RenderItem>>& allRitems)
{
}

void CBManager::UpdateObjectCBs(const GameTimer& gt)
{
    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    for (int i = 0; i < GeometryManager::mAllRitems.size(); ++i)
    {
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (GeometryManager::mAllRitems[i]->NumFramesDirty > 0)
        {
            DirectX::XMMATRIX world = XMLoadFloat4x4(&GeometryManager::mAllRitems[i]->World);
            DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
            ObjectConstants objConstants;

            if (i == 0)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 1.0f, 0.0f, 0.0f }, Obj1rotX)
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, Obj1rotY)
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 0.0f, 1.0f }, Obj1rotZ)
                * DirectX::XMMatrixTranslation(Obj1posX, Obj1posY, Obj1posZ);
            else if (i == 1)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 1.0f, 0.0f, 0.0f }, Obj2rotX)
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, Obj2rotY)
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 0.0f, 1.0f }, Obj2rotZ)
                * DirectX::XMMatrixTranslation(Obj2posX, Obj2posY, Obj2posZ);
            else if (i == 2 && activeSceneID != 11)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(10.0f, 0.0f, 0.0f);
            else if (i == 2)
                world = XMLoadFloat4x4(&worldM);
            else if (i == 3)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, 5.0f);
            else if (i == 4)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
            else if (i == 5)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos1[0], lightPos1[1], lightPos1[2]);
            else if (i == 6)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos2[0], lightPos2[1], lightPos2[2]);
            else if (i == 7)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos3[0], lightPos3[1], lightPos3[2]);
            else if (i == 9)
                world = XMLoadFloat4x4(&worldM)
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, DirectX::XM_PI / 2)
                * DirectX::XMMatrixRotationRollPitchYaw(spotLight1Direction[0], spotLight1Direction[1], spotLight1Direction[2])
                * DirectX::XMMatrixTranslation(lightPosSpot1[0], lightPosSpot1[1], lightPosSpot1[2]);
            else if (i >= 14 && i <= 742)
            {
                world = XMLoadFloat4x4(&worldM) * instanceDataScene3[i - 14];
            }
            else if (i == 743)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(-2.0f, 0.0f, 0.0f);
            }
            else if (i == 744)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(2.0f, 0.0f, 0.0f);
                GeometryManager::mAllRitems[i]->Mat->TilesCount = tilesCountScene4;
                GeometryManager::mAllRitems[i]->Mat->NumFramesDirty = gNumFrameResources;
            }
            if (activeSceneID == 10)
            {
                if (i == 751)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos1Scene10[0], lightPos1Scene10[1], lightPos1Scene10[2]);
                    objConstants.scale = light1DistanceScene10;
                }
                else if (i == 752)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos2Scene10[0], lightPos2Scene10[1], lightPos2Scene10[2]);
                    objConstants.scale = light2DistanceScene10;
                }
                else if (i == 753)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos3Scene10[0], lightPos3Scene10[1], lightPos3Scene10[2]);
                    objConstants.scale = light3DistanceScene10;
                }
                else if (i == 754)
                {
                    world = XMLoadFloat4x4(&worldM)
                        * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, DirectX::XM_PI / 2)
                        * DirectX::XMMatrixRotationRollPitchYaw(spotLight1DirectionScene10[0], spotLight1DirectionScene10[1], spotLight1DirectionScene10[2])
                        * DirectX::XMMatrixTranslation(lightPosSpot1Scene10[0], lightPosSpot1Scene10[1], lightPosSpot1Scene10[2]);
                    objConstants.scale = light1SpotDistanceScene10;
                }
            }
            else if (activeSceneID == 7)
            {
                if (i == 755)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 1.0f, 0.0f, 0.0f }, gt.TotalTime())
                        * DirectX::XMMatrixTranslation(10.0f, 5.0f, 0.f);
                }
                else if (i == 756)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, gt.TotalTime())
                        * DirectX::XMMatrixTranslation(0.0f, 5.0f, 0.f);
                }
                else if (i == 757)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 0.0f, 1.0f }, gt.TotalTime())
                        * DirectX::XMMatrixTranslation(-10.0f, 5.0f, 0.f);
                }
            }
            else if (activeSceneID == 9)
            {
                if (i == 758)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(10.0f, 0.0f, 0.0f);
                }
                else if (i == 759)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(5.0f, 0.0f, 0.0f);
                }
                else if (i == 760)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
                }
                else if (i == 761)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(-5.0f, 0.0f, 0.0f);
                }
                else if (i == 762)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(-10.0f, 0.0f, 0.0f);
                }
            }
            else if (activeSceneID == 3)
            {
                if (i == 763)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, -10.0f);
                }
                else if (i == 764)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, -10.0f);
                }
                else if (i == 765)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, -10.0f);
                }
                else if (i == 766)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(0.0f, 0.0f, -10.0f);
                }
            }
            else if (activeSceneID == 13)
            {
                if (i == 770)
                {
                    world = XMLoadFloat4x4(&worldM);
                }
            }
            else if (activeSceneID == 15)
            {
                if (i == 774)
                {
                    world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(
                        m_Scene15ObjectPostion.x,
                        m_Scene15ObjectPostion.y,
                        m_Scene15ObjectPostion.z
                    );
                }
            }

            DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&GeometryManager::mAllRitems[i]->TexTransform);

            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
            if (i <= 4 || i == 745)
                objConstants.isTessellationNeeded = 1.0f;
            else objConstants.isTessellationNeeded = 0.0f;
            if (i == 0)
                objConstants.scale = Obj1Scale;
            else if (i == 5)
                objConstants.scale = light1Distance;
            else if (i == 6)
                objConstants.scale = light2Distance;
            else if (i == 7)
                objConstants.scale = light3Distance;
            else if (i == 9)
                objConstants.scale = light1SpotDistance;

            currObjectCB->CopyData(GeometryManager::mAllRitems[i]->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            GeometryManager::mAllRitems[i]->NumFramesDirty--;
        }
    }
}

void CBManager::UpdateLightObjectCBs(const GameTimer& gt)
{
    auto currObjectCB = mCurrFrameResource->LightObjectCB.get();
    for (int i = 751; i <= 754; ++i)
    {
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (GeometryManager::mAllRitems[i]->NumFramesDirty > 0)
        {
            DirectX::XMMATRIX world = XMLoadFloat4x4(&GeometryManager::mAllRitems[i]->World);
            DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
            LightObjectConstants objConstants;
            if (i == 751)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos1Scene10[0], lightPos1Scene10[1], lightPos1Scene10[2]);
                objConstants.scale = light1DistanceScene10;
                objConstants.lightID = 3;
                objConstants.lightTypeID = 1;
            }
            else if (i == 752)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos2Scene10[0], lightPos2Scene10[1], lightPos2Scene10[2]);
                objConstants.scale = light2DistanceScene10;
                objConstants.lightID = 4;
                objConstants.lightTypeID = 1;
            }
            else if (i == 753)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(lightPos3Scene10[0], lightPos3Scene10[1], lightPos3Scene10[2]);
                objConstants.scale = light3DistanceScene10;
                objConstants.lightID = 5;
                objConstants.lightTypeID = 1;
            }
            else if (i == 754)
            {
                world = XMLoadFloat4x4(&worldM)
                    * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, DirectX::XM_PI / 2)
                    * DirectX::XMMatrixRotationRollPitchYaw(spotLight1DirectionScene10[0], spotLight1DirectionScene10[1], spotLight1DirectionScene10[2])
                    * DirectX::XMMatrixTranslation(lightPosSpot1Scene10[0], lightPosSpot1Scene10[1], lightPosSpot1Scene10[2]);
                objConstants.scale = light1SpotDistanceScene10;
                objConstants.lightID = 6;
                objConstants.lightTypeID = 2;
            }
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

            currObjectCB->CopyData(GeometryManager::mAllRitems[i]->ObjCBIndex - 751, objConstants);

            // Next FrameResource need to be updated too.
            GeometryManager::mAllRitems[i]->NumFramesDirty--;
        }
    }
}

void CBManager::UpdateLightObjectCBMoreLight(const GameTimer& gt)
{
    auto currObjectCB = mCurrFrameResource->LightObjectCB.get();
    // Only update the cbuffer data if the constants have changed.  
    // This needs to be tracked per frame resource.
    if (GeometryManager::mAllRitems[769]->NumFramesDirty > 0)
    {
        DirectX::XMMATRIX world = XMLoadFloat4x4(&GeometryManager::mAllRitems[769]->World);
        DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
        LightObjectConstants objConstants;
        {
            world = XMLoadFloat4x4(&worldM);
            objConstants.scale = moreLightDistanceScene10;
            objConstants.lightID = 0;
            objConstants.lightTypeID = 0;
        }
        XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

        currObjectCB->CopyData(4, objConstants);

        // Next FrameResource need to be updated too.
        GeometryManager::mAllRitems[769]->NumFramesDirty--;
    }
}

void CBManager::UpdateTAAObjectCBs(const GameTimer& gt)
{
    auto currObjectCB = mCurrFrameResource->TAAObjectsCB.get();
    for (int i = 0; i < GeometryManager::mAllRitems.size(); ++i)
    {
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (GeometryManager::mAllRitems[i]->NumFramesDirty > 0)
        {
            DirectX::XMMATRIX world = XMLoadFloat4x4(&GeometryManager::mAllRitems[i]->World);
            DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
            TAAObjectConstants objConstants;

            if (i == 774)
            {
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(
                    m_Scene15ObjectPostion.x,
                    m_Scene15ObjectPostion.y,
                    m_Scene15ObjectPostion.z
                );
            }

            DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&GeometryManager::mAllRitems[i]->TexTransform);

            DirectX::XMMATRIX prevWorld = XMLoadFloat4x4(&PreviousWorld);
            XMStoreFloat4x4(&objConstants.PreviousWorld, XMMatrixTranspose(prevWorld));

            DirectX::XMStoreFloat4x4(&PreviousWorld, world);

            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

            currObjectCB->CopyData(GeometryManager::mAllRitems[i]->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            GeometryManager::mAllRitems[i]->NumFramesDirty--;
        }
    }
}

void CBManager::UpdateMaterialCBs(const GameTimer& gt)
{
    auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
    for (auto& e : (GeometryManager::mMaterials))
    {
        // Only update the cbuffer data if the constants have changed.  If the cbuffer
        // data changes, it needs to be updated for each FrameResource.
        Material* mat = e.second.get();
        if (mat->NumFramesDirty > 0)
        {
            DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

            MaterialConstants matConstants;
            matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
            matConstants.FresnelR0 = mat->FresnelR0;
            matConstants.Roughness = mat->Roughness;
            matConstants.TilesCount = mat->TilesCount;
            if (mat->IsMetallic == 1)
                mat->TilesCount = 2;
            XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

            currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

            // Next FrameResource need to be updated too.
            mat->NumFramesDirty--;
        }
    }
}

void CBManager::UpdateMainPassCB(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };


    // Point lights
    {
        mMainPassCB.Lights[3].Strength = { light1Strength, light1Strength, light1Strength };
        mMainPassCB.Lights[3].Position = { lightPos1[0], lightPos1[1], lightPos1[2] };
        mMainPassCB.Lights[3].FalloffStart = light1Distance;
        mMainPassCB.Lights[3].FalloffEnd = light1Distance;
        mMainPassCB.Lights[3].Color = { col1[0], col1[1], col1[2], 1.0f };

        mMainPassCB.Lights[4].Strength = { light2Strength, light2Strength, light2Strength };
        mMainPassCB.Lights[4].Position = { lightPos2[0], lightPos2[1], lightPos2[2] };
        mMainPassCB.Lights[4].FalloffStart = light2Distance;
        mMainPassCB.Lights[4].FalloffEnd = light2Distance;
        mMainPassCB.Lights[4].Color = { col2[0], col2[1], col2[2], 1.0f };

        mMainPassCB.Lights[5].Strength = { light3Strength, light3Strength, light3Strength };
        mMainPassCB.Lights[5].Position = { lightPos3[0], lightPos3[1], lightPos3[2] };
        mMainPassCB.Lights[5].FalloffStart = light3Distance;
        mMainPassCB.Lights[5].FalloffEnd = light3Distance;
        mMainPassCB.Lights[5].Color = { col3[0], col3[1], col3[2], 1.0f };
    }


    // Spot lights
    mMainPassCB.Lights[6].Strength = { light1SpotStrength, light1SpotStrength, light1SpotStrength };
    mMainPassCB.Lights[6].FalloffStart = light1SpotDistance;
    mMainPassCB.Lights[6].FalloffEnd = light1SpotDistance;
    mMainPassCB.Lights[6].Color = { colSpot1[0], colSpot1[1], colSpot1[2], 1.0f };
    mMainPassCB.Lights[6].SpotPower = spotLight1Power;
    DirectX::XMVECTOR vector = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(spotLight1Direction[0], spotLight1Direction[1], spotLight1Direction[2]);
    DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Transform(vector, rotation);
    DirectX::XMStoreFloat3(&mMainPassCB.Lights[6].Direction, rotatedVector);
    mMainPassCB.Lights[6].Position = { lightPosSpot1[0], lightPosSpot1[1], lightPosSpot1[2] };

    if (flashlightIsActive)
    {
        mMainPassCB.Lights[7].Strength = { light2SpotStrength, light2SpotStrength, light2SpotStrength };
        mMainPassCB.Lights[7].FalloffStart = light2SpotDistance;
        mMainPassCB.Lights[7].FalloffEnd = light2SpotDistance;
        mMainPassCB.Lights[7].SpotPower = spotLight2Power;
        mMainPassCB.Lights[7].Color = { colSpot2[0], colSpot2[1], colSpot2[2], 1.0f };
        mMainPassCB.Lights[7].Direction = (*mCamera).GetLook3f();
        mMainPassCB.Lights[7].Position = (*mCamera).GetPosition3f();
    }
    else mMainPassCB.Lights[7].Strength = { 0.0f, 0.0f, 0.0f };

    if (activeSceneID == 1)
    {
        mMainPassCB.TessFactor = tessFactor;
        mMainPassCB.displacementLevel = displacementLevel;
    }
    else if (activeSceneID == 5)
    {
        mMainPassCB.TessFactor = tessFactorScene5;
        mMainPassCB.displacementLevel = displacementLevelScene5;
    }

    mMainPassCB.PixelationFactor = pixelationFactor;
    if (isParallaxMapping)
        mMainPassCB.ParallaxMapping = 1.0f;
    else mMainPassCB.ParallaxMapping = 0.0f;


    if (isNegative)
        mMainPassCB.isNegative = 1.0f;
    else mMainPassCB.isNegative = 0.0f;

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBScene10(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };


    // Point lights
    {
        mMainPassCB.Lights[3].Strength = { light1StrengthScene10, light1StrengthScene10, light1StrengthScene10 };
        mMainPassCB.Lights[3].Position = { lightPos1Scene10[0], lightPos1Scene10[1], lightPos1Scene10[2] };
        mMainPassCB.Lights[3].FalloffStart = light1DistanceScene10;
        mMainPassCB.Lights[3].FalloffEnd = light1DistanceScene10;
        mMainPassCB.Lights[3].Color = { col1Scene10[0], col1Scene10[1], col1Scene10[2], 1.0f };

        mMainPassCB.Lights[4].Strength = { light2StrengthScene10, light2StrengthScene10, light2StrengthScene10 };
        mMainPassCB.Lights[4].Position = { lightPos2Scene10[0], lightPos2Scene10[1], lightPos2Scene10[2] };
        mMainPassCB.Lights[4].FalloffStart = light2DistanceScene10;
        mMainPassCB.Lights[4].FalloffEnd = light2DistanceScene10;
        mMainPassCB.Lights[4].Color = { col2Scene10[0], col2Scene10[1], col2Scene10[2], 1.0f };

        mMainPassCB.Lights[5].Strength = { light3StrengthScene10, light3StrengthScene10, light3StrengthScene10 };
        mMainPassCB.Lights[5].Position = { lightPos3Scene10[0], lightPos3Scene10[1], lightPos3Scene10[2] };
        mMainPassCB.Lights[5].FalloffStart = light3DistanceScene10;
        mMainPassCB.Lights[5].FalloffEnd = light3DistanceScene10;
        mMainPassCB.Lights[5].Color = { col3Scene10[0], col3Scene10[1], col3Scene10[2], 1.0f };
    }


    // Spot lights
    mMainPassCB.Lights[6].Strength = { light1SpotStrengthScene10, light1SpotStrengthScene10, light1SpotStrengthScene10 };
    mMainPassCB.Lights[6].FalloffStart = light1SpotDistanceScene10;
    mMainPassCB.Lights[6].FalloffEnd = light1SpotDistanceScene10;
    mMainPassCB.Lights[6].Color = { colSpot1Scene10[0], colSpot1Scene10[1], colSpot1Scene10[2], 1.0f };
    mMainPassCB.Lights[6].SpotPower = spotLight1PowerScene10;
    DirectX::XMVECTOR vector = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(spotLight1DirectionScene10[0], spotLight1DirectionScene10[1], spotLight1DirectionScene10[2]);
    DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Transform(vector, rotation);
    DirectX::XMStoreFloat3(&mMainPassCB.Lights[6].Direction, rotatedVector);
    mMainPassCB.Lights[6].Position = { lightPosSpot1Scene10[0], lightPosSpot1Scene10[1], lightPosSpot1Scene10[2] };

    if (activeSceneID == 1)
    {
        mMainPassCB.TessFactor = tessFactor;
        mMainPassCB.displacementLevel = displacementLevel;
    }
    else if (activeSceneID == 5)
    {
        mMainPassCB.TessFactor = tessFactorScene5;
        mMainPassCB.displacementLevel = displacementLevelScene5;
    }

    mMainPassCB.PixelationFactor = pixelationFactor;
    if (isParallaxMapping)
        mMainPassCB.ParallaxMapping = 1.0f;
    else mMainPassCB.ParallaxMapping = 0.0f;


    if (isNegative)
        mMainPassCB.isNegative = 1.0f;
    else mMainPassCB.isNegative = 0.0f;

    mMainPassCB.Resolution = DirectX::XMFLOAT2(1280, 800);

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBMoreLightScene10(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };


    // Point lights
    {
        mMainPassCB.Lights[3].Strength = { light1StrengthScene10, light1StrengthScene10, light1StrengthScene10 };
        mMainPassCB.Lights[3].Position = { lightPos1Scene10[0], lightPos1Scene10[1], lightPos1Scene10[2] };
        mMainPassCB.Lights[3].FalloffStart = light1DistanceScene10;
        mMainPassCB.Lights[3].FalloffEnd = light1DistanceScene10;
        mMainPassCB.Lights[3].Color = { col1Scene10[0], col1Scene10[1], col1Scene10[2], 1.0f };

        mMainPassCB.Lights[4].Strength = { light2StrengthScene10, light2StrengthScene10, light2StrengthScene10 };
        mMainPassCB.Lights[4].Position = { lightPos2Scene10[0], lightPos2Scene10[1], lightPos2Scene10[2] };
        mMainPassCB.Lights[4].FalloffStart = light2DistanceScene10;
        mMainPassCB.Lights[4].FalloffEnd = light2DistanceScene10;
        mMainPassCB.Lights[4].Color = { col2Scene10[0], col2Scene10[1], col2Scene10[2], 1.0f };

        mMainPassCB.Lights[5].Strength = { light3StrengthScene10, light3StrengthScene10, light3StrengthScene10 };
        mMainPassCB.Lights[5].Position = { lightPos3Scene10[0], lightPos3Scene10[1], lightPos3Scene10[2] };
        mMainPassCB.Lights[5].FalloffStart = light3DistanceScene10;
        mMainPassCB.Lights[5].FalloffEnd = light3DistanceScene10;
        mMainPassCB.Lights[5].Color = { col3Scene10[0], col3Scene10[1], col3Scene10[2], 1.0f };
    }
    for (int i = 6; i < 506; ++i)
    {
        mMainPassCB.Lights[i].Strength = { moreLightStrengthScene10, moreLightStrengthScene10, moreLightStrengthScene10 };
        mMainPassCB.Lights[i].Position = { float((((i - 6) / 100)) * 3), float((((i - 6) % 10)) * 2), float(((((i - 6) % 100) / 10)) * 3) };
        mMainPassCB.Lights[i].FalloffStart = moreLightDistanceScene10;
        mMainPassCB.Lights[i].FalloffEnd = moreLightDistanceScene10;
        mMainPassCB.Lights[i].Color = { moreLightColScene10[0], moreLightColScene10[1], moreLightColScene10[2], 1.0f };
    }


    // Spot lights
    mMainPassCB.Lights[506].Strength = { light1SpotStrengthScene10, light1SpotStrengthScene10, light1SpotStrengthScene10 };
    mMainPassCB.Lights[506].FalloffStart = light1SpotDistanceScene10;
    mMainPassCB.Lights[506].FalloffEnd = light1SpotDistanceScene10;
    mMainPassCB.Lights[506].Color = { colSpot1Scene10[0], colSpot1Scene10[1], colSpot1Scene10[2], 1.0f };
    mMainPassCB.Lights[506].SpotPower = spotLight1PowerScene10;
    DirectX::XMVECTOR vector = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(spotLight1DirectionScene10[0], spotLight1DirectionScene10[1], spotLight1DirectionScene10[2]);
    DirectX::XMVECTOR rotatedVector = DirectX::XMVector3Transform(vector, rotation);
    DirectX::XMStoreFloat3(&mMainPassCB.Lights[6].Direction, rotatedVector);
    mMainPassCB.Lights[506].Position = { lightPosSpot1Scene10[0], lightPosSpot1Scene10[1], lightPosSpot1Scene10[2] };

    if (activeSceneID == 1)
    {
        mMainPassCB.TessFactor = tessFactor;
        mMainPassCB.displacementLevel = displacementLevel;
    }
    else if (activeSceneID == 5)
    {
        mMainPassCB.TessFactor = tessFactorScene5;
        mMainPassCB.displacementLevel = displacementLevelScene5;
    }

    mMainPassCB.PixelationFactor = pixelationFactor;
    if (isParallaxMapping)
        mMainPassCB.ParallaxMapping = 1.0f;
    else mMainPassCB.ParallaxMapping = 0.0f;


    if (isNegative)
        mMainPassCB.isNegative = 1.0f;
    else mMainPassCB.isNegative = 0.0f;

    mMainPassCB.Resolution = DirectX::XMFLOAT2(1280, 800);

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBParticles(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = mRotatedLightDirections[0];
    mMainPassCB.Lights[0].Strength = { 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = mRotatedLightDirections[1];
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };

    mMainPassCB.Lights[2].Direction = mRotatedLightDirections[2];
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    // Point lights
    mMainPassCB.Lights[3].Strength = { 1.0f, 1.0f, 1.0f };
    mMainPassCB.Lights[3].Position = { 1.0f, 5.0f, 0.0f };
    mMainPassCB.Lights[3].FalloffStart = 5.f;
    mMainPassCB.Lights[3].FalloffEnd = 5.f;
    mMainPassCB.Lights[3].Color = { 1.0f, 0.0f, 0.0f, 1.0f };

    mMainPassCB.Lights[4].Strength = { 1.0f, 1.0f, 1.0f };
    mMainPassCB.Lights[4].Position = { -1.0f, 8.0f, 0.0f };
    mMainPassCB.Lights[4].FalloffStart = 5.f;
    mMainPassCB.Lights[4].FalloffEnd = 5.f;
    mMainPassCB.Lights[4].Color = { 0.0f, 1.0f, 0.0f, 1.0f };

    mMainPassCB.Lights[5].Strength = { 1.0f, 1.0f, 1.0f };
    mMainPassCB.Lights[5].Position = { 0.0f, 3.0f, -1.0f };
    mMainPassCB.Lights[5].FalloffStart = 5.f;
    mMainPassCB.Lights[5].FalloffEnd = 5.f;
    mMainPassCB.Lights[5].Color = { 0.0f, 0.0f, 1.0f, 1.0f };

    if (activeSceneID == 1)
    {
        mMainPassCB.TessFactor = tessFactor;
        mMainPassCB.displacementLevel = displacementLevel;
    }
    else if (activeSceneID == 5)
    {
        mMainPassCB.TessFactor = tessFactorScene5;
        mMainPassCB.displacementLevel = displacementLevelScene5;
    }

    mMainPassCB.PixelationFactor = pixelationFactor;
    if (isParallaxMapping)
        mMainPassCB.ParallaxMapping = 1.0f;
    else mMainPassCB.ParallaxMapping = 0.0f;

    if (isNegative)
        mMainPassCB.isNegative = 1.0f;
    else mMainPassCB.isNegative = 0.0f;
    mMainPassCB.lightingID = lightingIDScene6;

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBScene3Camera2(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera2Scene3).GetView();
    DirectX::XMMATRIX proj = (*mCamera2Scene3).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera2Scene3).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto currPassCB = mCurrFrameResource->PassCBScene3Camera2.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBScene13Camera2(const GameTimer& gt)
{
    PassConstants mMainPassCB;

    DirectX::XMMATRIX view = (*mCamera2Scene13).GetView();
    DirectX::XMMATRIX proj = (*mCamera2Scene13).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mMainPassCB.EyePosW = (*mCamera2Scene13).GetPosition3f();
    mMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1000.0f;
    mMainPassCB.TotalTime = gt.TotalTime();
    mMainPassCB.DeltaTime = gt.DeltaTime();
    mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto currPassCB = mCurrFrameResource->PassCBScene13Camera2.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void CBManager::UpdateMainPassCBShadows(const GameTimer& gt)
{
    PassConstantsShadows mMainPassCBShadows;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);
    DirectX::XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

    XMStoreFloat4x4(&mMainPassCBShadows.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCBShadows.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCBShadows.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCBShadows.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCBShadows.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCBShadows.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&mMainPassCBShadows.ShadowTransform, XMMatrixTranspose(shadowTransform));
    mMainPassCBShadows.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCBShadows.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCBShadows.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCBShadows.NearZ = 1.0f;
    mMainPassCBShadows.FarZ = 1000.0f;
    mMainPassCBShadows.TotalTime = gt.TotalTime();
    mMainPassCBShadows.DeltaTime = gt.DeltaTime();
    mMainPassCBShadows.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCBShadows.Lights[0].Direction = mRotatedLightDirections[0];
    mMainPassCBShadows.Lights[0].Strength = { 1.0f, 1.0f, 1.0f };

    mMainPassCBShadows.Lights[1].Direction = mRotatedLightDirections[1];
    mMainPassCBShadows.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };

    mMainPassCBShadows.Lights[2].Direction = mRotatedLightDirections[2];
    mMainPassCBShadows.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    if (activeSceneID == 6 && activeParticleSystemScene6 == 3)
    {
        mMainPassCBShadows.Lights[3].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[3].Position = { 1.0f, 5.0f, 0.0f };
        mMainPassCBShadows.Lights[3].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[3].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[3].Color = { 1.0f, 0.0f, 0.0f, 1.0f };

        mMainPassCBShadows.Lights[4].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[4].Position = { -1.0f, 8.0f, 0.0f };
        mMainPassCBShadows.Lights[4].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[4].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[4].Color = { 0.0f, 1.0f, 0.0f, 1.0f };

        mMainPassCBShadows.Lights[5].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[5].Position = { 0.0f, 1.0f, -3.0f };
        mMainPassCBShadows.Lights[5].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[5].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[5].Color = { 0.0f, 0.0f, 1.0f, 1.0f };
    }

    mMainPassCBShadows.ShadowTextureID = shadowTextureIDScene7;
    if (isTexturedShadowsScene7)
        mMainPassCBShadows.BIsTexturedShadows = 1;
    else mMainPassCBShadows.BIsTexturedShadows = 0;

    mMainPassCBShadows.ShadowSizeID = shadowSizeIDScene7;
    if (isUsingCascadedShadowsScene7)
        mMainPassCBShadows.BIsCascadedShadows = 1;
    else mMainPassCBShadows.BIsCascadedShadows = 0;
    mMainPassCBShadows.ShadowFilteringID = shadowFilteringIDScene7;

    auto currPassCB = mCurrFrameResource->PassCBShadows.get();
    currPassCB->CopyData(0, mMainPassCBShadows);
}

void CBManager::UpdateMainPassCBShadowsCascaded(const GameTimer& gt)
{
    PassConstantsShadows mMainPassCBShadows;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);
    DirectX::XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform256);

    XMStoreFloat4x4(&mMainPassCBShadows.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCBShadows.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCBShadows.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCBShadows.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCBShadows.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCBShadows.InvViewProj, XMMatrixTranspose(invViewProj));
    XMStoreFloat4x4(&mMainPassCBShadows.ShadowTransform, XMMatrixTranspose(shadowTransform));
    mMainPassCBShadows.EyePosW = (*mCamera).GetPosition3f();
    mMainPassCBShadows.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mMainPassCBShadows.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mMainPassCBShadows.NearZ = 1.0f;
    mMainPassCBShadows.FarZ = 1000.0f;
    mMainPassCBShadows.TotalTime = gt.TotalTime();
    mMainPassCBShadows.DeltaTime = gt.DeltaTime();
    mMainPassCBShadows.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mMainPassCBShadows.Lights[0].Direction = mRotatedLightDirections[0];
    mMainPassCBShadows.Lights[0].Strength = { 1.0f, 1.0f, 1.0f };

    mMainPassCBShadows.Lights[1].Direction = mRotatedLightDirections[1];
    mMainPassCBShadows.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };

    mMainPassCBShadows.Lights[2].Direction = mRotatedLightDirections[2];
    mMainPassCBShadows.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };

    if (activeSceneID == 6 && activeParticleSystemScene6 == 3)
    {
        mMainPassCBShadows.Lights[3].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[3].Position = { 1.0f, 5.0f, 0.0f };
        mMainPassCBShadows.Lights[3].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[3].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[3].Color = { 1.0f, 0.0f, 0.0f, 1.0f };

        mMainPassCBShadows.Lights[4].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[4].Position = { -1.0f, 8.0f, 0.0f };
        mMainPassCBShadows.Lights[4].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[4].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[4].Color = { 0.0f, 1.0f, 0.0f, 1.0f };

        mMainPassCBShadows.Lights[5].Strength = { 1.0f, 1.0f, 1.0f };
        mMainPassCBShadows.Lights[5].Position = { 0.0f, 1.0f, -3.0f };
        mMainPassCBShadows.Lights[5].FalloffStart = 5.f;
        mMainPassCBShadows.Lights[5].FalloffEnd = 7.f;
        mMainPassCBShadows.Lights[5].Color = { 0.0f, 0.0f, 1.0f, 1.0f };
    }

    mMainPassCBShadows.ShadowTextureID = shadowTextureIDScene7;
    if (isTexturedShadowsScene7)
        mMainPassCBShadows.BIsTexturedShadows = 1;
    else mMainPassCBShadows.BIsTexturedShadows = 0;

    mMainPassCBShadows.ShadowSizeID = shadowSizeIDScene7;
    if (isUsingCascadedShadowsScene7)
        mMainPassCBShadows.BIsCascadedShadows = 1;
    else mMainPassCBShadows.BIsCascadedShadows = 0;
    mMainPassCBShadows.ShadowFilteringID = shadowFilteringIDScene7;

    auto currPassCB = mCurrFrameResource->PassCBShadowsCascaded.get();
    currPassCB->CopyData(0, mMainPassCBShadows);

    shadowTransform = XMLoadFloat4x4(&mShadowTransform512);
    XMStoreFloat4x4(&mMainPassCBShadows.ShadowTransform, XMMatrixTranspose(shadowTransform));

    currPassCB = mCurrFrameResource->PassCBShadowsCascaded.get();
    currPassCB->CopyData(1, mMainPassCBShadows);

    shadowTransform = XMLoadFloat4x4(&mShadowTransform1024);
    XMStoreFloat4x4(&mMainPassCBShadows.ShadowTransform, XMMatrixTranspose(shadowTransform));

    currPassCB = mCurrFrameResource->PassCBShadowsCascaded.get();
    currPassCB->CopyData(2, mMainPassCBShadows);

    shadowTransform = XMLoadFloat4x4(&mShadowTransform2048);
    XMStoreFloat4x4(&mMainPassCBShadows.ShadowTransform, XMMatrixTranspose(shadowTransform));

    currPassCB = mCurrFrameResource->PassCBShadowsCascaded.get();
    currPassCB->CopyData(3, mMainPassCBShadows);
}

void CBManager::UpdateShadowPassCB(const GameTimer& gt)
{
    PassConstantsShadows mShadowPassCB;

    DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mLightView);
    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&mLightProj);

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);

    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);

    UINT w = mShadowMap2048->Width();
    UINT h = mShadowMap2048->Height();
    if (!isUsingCascadedShadowsScene7)
    {
        if (shadowSizeIDScene7 == 3)
        {
            w = mShadowMap2048->Width();
            h = mShadowMap2048->Height();
        }
        else if (shadowSizeIDScene7 == 2)
        {
            w = mShadowMap1024->Width();
            h = mShadowMap1024->Height();
        }
        else if (shadowSizeIDScene7 == 2)
        {
            w = mShadowMap512->Width();
            h = mShadowMap512->Height();
        }
        else
        {
            w = mShadowMap256->Width();
            h = mShadowMap256->Height();
        }
    }

    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.EyePosW = mLightPosW;
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB.NearZ = mLightNearZ;
    mShadowPassCB.FarZ = mLightFarZ;

    auto currPassCB = mCurrFrameResource->ShadowPassCB.get();
    currPassCB->CopyData(0, mShadowPassCB);
}

void CBManager::UpdateShadowPassCBCascaded(const GameTimer& gt)
{
    PassConstantsShadows mShadowPassCB;

    DirectX::XMMATRIX view = (*mCameraShadowMap256).GetView();
    DirectX::XMMATRIX proj = (*mCameraShadowMap256).GetProj();

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);

    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);

    UINT w = mShadowMap256->Width();
    UINT h = mShadowMap256->Height();

    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.EyePosW = mLightPosW;
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB.NearZ = mLightNearZ;
    mShadowPassCB.FarZ = mLightFarZ;

    auto currPassCB = mCurrFrameResource->ShadowPassCBCascaded.get();
    currPassCB->CopyData(0, mShadowPassCB);


    view = (*mCameraShadowMap512).GetView();
    proj = (*mCameraShadowMap512).GetProj();
    tmp1 = XMMatrixDeterminant(view);
    tmp2 = XMMatrixDeterminant(proj);
    viewProj = DirectX::XMMatrixMultiply(view, proj);
    tmp3 = XMMatrixDeterminant(viewProj);
    invView = DirectX::XMMatrixInverse(&tmp1, view);
    invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);
    w = mShadowMap512->Width();
    h = mShadowMap512->Height();
    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);

    currPassCB = mCurrFrameResource->ShadowPassCBCascaded.get();
    currPassCB->CopyData(1, mShadowPassCB);


    view = (*mCameraShadowMap1024).GetView();
    proj = (*mCameraShadowMap1024).GetProj();
    tmp1 = XMMatrixDeterminant(view);
    tmp2 = XMMatrixDeterminant(proj);
    viewProj = DirectX::XMMatrixMultiply(view, proj);
    tmp3 = XMMatrixDeterminant(viewProj);
    invView = DirectX::XMMatrixInverse(&tmp1, view);
    invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);
    w = mShadowMap1024->Width();
    h = mShadowMap1024->Height();
    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);

    currPassCB = mCurrFrameResource->ShadowPassCBCascaded.get();
    currPassCB->CopyData(2, mShadowPassCB);


    view = (*mCameraShadowMap2048).GetView();
    proj = (*mCameraShadowMap2048).GetProj();
    tmp1 = XMMatrixDeterminant(view);
    tmp2 = XMMatrixDeterminant(proj);
    viewProj = DirectX::XMMatrixMultiply(view, proj);
    tmp3 = XMMatrixDeterminant(viewProj);
    invView = DirectX::XMMatrixInverse(&tmp1, view);
    invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);
    w = mShadowMap2048->Width();
    h = mShadowMap2048->Height();
    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);

    currPassCB = mCurrFrameResource->ShadowPassCBCascaded.get();
    currPassCB->CopyData(3, mShadowPassCB);
}

void CBManager::UpdateShadowPassCBParticles(const GameTimer& gt)
{
    PassConstantsShadows mShadowPassCB;

    DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&mLightView);
    DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&mLightProj);

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);

    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = DirectX::XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(&tmp3, viewProj);

    UINT w = mShadowMap2048->Width();
    UINT h = mShadowMap2048->Height();

    DirectX::XMStoreFloat4x4(&mShadowPassCB.View, XMMatrixTranspose(view));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvView, XMMatrixTranspose(invView));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.Proj, XMMatrixTranspose(proj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvProj, XMMatrixTranspose(invProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
    DirectX::XMStoreFloat4x4(&mShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    mShadowPassCB.EyePosW = mLightPosW;
    mShadowPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB.NearZ = mLightNearZ;
    mShadowPassCB.FarZ = mLightFarZ;

    auto currPassCB = mCurrFrameResource->ShadowPassCBParticles.get();
    currPassCB->CopyData(0, mShadowPassCB);
}

void CBManager::UpdateShadowTransform(const GameTimer& gt)
{
    float sceneRadius = 50.f;
    DirectX::XMFLOAT3 sceneCenter(0.0f, 0.0f, 0.0f);
    // Only the first "main" light casts a shadow.
    DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&mRotatedLightDirections[0]);
    DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(lightDir, -2.0f * sceneRadius);
    DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&sceneCenter);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);

    DirectX::XMStoreFloat3(&mLightPosW, lightPos);

    // Transform bounding sphere to light space.
    DirectX::XMFLOAT3 sphereCenterLS;
    DirectX::XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - sceneRadius;
    float b = sphereCenterLS.y - sceneRadius;
    float n = sphereCenterLS.z - sceneRadius;
    float r = sphereCenterLS.x + sceneRadius;
    float t = sphereCenterLS.y + sceneRadius;
    float f = sphereCenterLS.z + sceneRadius;

    mLightNearZ = n;
    mLightFarZ = f;
    DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    DirectX::XMMATRIX S = lightView * lightProj * T;
    DirectX::XMStoreFloat4x4(&mLightView, lightView);
    DirectX::XMStoreFloat4x4(&mLightProj, lightProj);
    DirectX::XMStoreFloat4x4(&mShadowTransform, S);
}

void CBManager::UpdateMainPassCBScene15(const GameTimer& gt)
{
    TAAPassConstants mTAAMainPassCB;

    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();

    TAAUtility::GenerateJitter();
    proj = TAAUtility::GetJitterPerspectiveProjectionMatrix(
        (*mCamera).GetNearZ(),
        (*mCamera).GetFarZ(),
        (*mCamera).GetFovY(),
        (*mCamera).GetAspect(),
        proj
    );

    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&mTAAMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mTAAMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mTAAMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mTAAMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mTAAMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mTAAMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

    DirectX::XMMATRIX prevView = DirectX::XMLoadFloat4x4(&PreviousView);
    DirectX::XMMATRIX prevProj = DirectX::XMLoadFloat4x4(&PreviousProj);

    auto tmp4 = XMMatrixDeterminant(prevView);
    auto tmp5 = XMMatrixDeterminant(prevProj);
    DirectX::XMMATRIX prevViewProj = XMMatrixMultiply(prevView, prevProj);
    auto tmp6 = XMMatrixDeterminant(prevViewProj);
    DirectX::XMMATRIX prevInvView = XMMatrixInverse(&tmp4, prevView);
    DirectX::XMMATRIX prevInvProj = XMMatrixInverse(&tmp5, prevProj);
    DirectX::XMMATRIX prevInvViewProj = XMMatrixInverse(&tmp6, prevViewProj);

    XMStoreFloat4x4(&mTAAMainPassCB.PrevView, XMMatrixTranspose(prevView));
    XMStoreFloat4x4(&mTAAMainPassCB.PrevInvView, XMMatrixTranspose(prevInvView));
    XMStoreFloat4x4(&mTAAMainPassCB.PrevProj, XMMatrixTranspose(prevProj));
    XMStoreFloat4x4(&mTAAMainPassCB.PrevInvProj, XMMatrixTranspose(prevInvProj));
    XMStoreFloat4x4(&mTAAMainPassCB.PrevViewProj, XMMatrixTranspose(prevViewProj));
    XMStoreFloat4x4(&mTAAMainPassCB.PrevInvViewProj, XMMatrixTranspose(prevInvViewProj));

    mTAAMainPassCB.EyePosW = (*mCamera).GetPosition3f();
    mTAAMainPassCB.RenderTargetSize = DirectX::XMFLOAT2((float)mClientWidth, (float)mClientHeight);
    mTAAMainPassCB.InvRenderTargetSize = DirectX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
    mTAAMainPassCB.NearZ = 1.0f;
    mTAAMainPassCB.FarZ = 1000.0f;
    mTAAMainPassCB.TotalTime = gt.TotalTime();
    mTAAMainPassCB.DeltaTime = gt.DeltaTime();
    mTAAMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

    // Directional lights
    mTAAMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
    mTAAMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.8f };
    mTAAMainPassCB.Lights[0].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mTAAMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
    mTAAMainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
    mTAAMainPassCB.Lights[1].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    mTAAMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
    mTAAMainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };
    mTAAMainPassCB.Lights[2].Color = { 1.0f, 1.0f, 1.0f, 1.0f };

    auto currPassCB = mCurrFrameResource->TAAPassCB.get();
    currPassCB->CopyData(0, mTAAMainPassCB);

    DirectX::XMStoreFloat4x4(&PreviousView, view);
    DirectX::XMStoreFloat4x4(&PreviousProj, proj);
}

void CBManager::UpdateShadowTransformCascaded(Camera camera, int MapID, DirectX::XMFLOAT3 center,
    float l, float r, float b, float t, float n, float f)
{
    float sceneRadius = 50.f;
    // Only the first "main" light casts a shadow.
    DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&mRotatedLightDirections[0]);
    DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(lightDir, -2.0f * sceneRadius);
    DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPos, targetPos, lightUp);

    DirectX::XMStoreFloat3(&mLightPosW, lightPos);

    // Transform bounding sphere to light space.
    DirectX::XMFLOAT3 sphereCenterLS;
    DirectX::XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

    // Ortho frustum in light space encloses scene.
    mLightNearZ = n;
    mLightFarZ = f;
    DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);

    DirectX::XMMATRIX S = lightView * lightProj * T;
    if (MapID == 3)
    {
        DirectX::XMStoreFloat4x4(&mLightView2048, lightView);
        DirectX::XMStoreFloat4x4(&mLightProj2048, lightProj);
        DirectX::XMStoreFloat4x4(&mShadowTransform2048, S);
    }
    if (MapID == 2)
    {
        DirectX::XMStoreFloat4x4(&mLightView1024, lightView);
        DirectX::XMStoreFloat4x4(&mLightProj1024, lightProj);
        DirectX::XMStoreFloat4x4(&mShadowTransform1024, S);
    }
    if (MapID == 1)
    {
        DirectX::XMStoreFloat4x4(&mLightView512, lightView);
        DirectX::XMStoreFloat4x4(&mLightProj512, lightProj);
        DirectX::XMStoreFloat4x4(&mShadowTransform512, S);
    }
    if (MapID == 0)
    {
        DirectX::XMStoreFloat4x4(&mLightView256, lightView);
        DirectX::XMStoreFloat4x4(&mLightProj256, lightProj);
        DirectX::XMStoreFloat4x4(&mShadowTransform256, S);
    }
}

void CBManager::UpdateParticleEmitterCB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;

    mParticleSystem->emitterData.StartColor = DirectX::XMFLOAT4(particleStartColorScene6[0], particleStartColorScene6[1], particleStartColorScene6[2], 1.0f);
    mParticleSystem->emitterData.EndColor = DirectX::XMFLOAT4(particleEndColorScene6[0], particleEndColorScene6[1], particleEndColorScene6[2], 1.0f);
    mParticleSystem->emitterData.StartSize = particleStartSizeScene6;
    mParticleSystem->emitterData.EndSize = particleEndSizeScene6;

    emitterConstPass.Position = mParticleSystem->emitterData.Position;
    emitterConstPass.DeltaTime = gt.DeltaTime();
    emitterConstPass.GravityForce = mParticleSystem->emitterData.GravityForce;
    emitterConstPass.SystemID = mParticleSystem->emitterData.SystemID;
    emitterConstPass.StartColor = mParticleSystem->emitterData.StartColor;
    emitterConstPass.EndColor = mParticleSystem->emitterData.EndColor;
    emitterConstPass.StartSize = mParticleSystem->emitterData.StartSize;
    emitterConstPass.EndSize = mParticleSystem->emitterData.EndSize;
    emitterConstPass.MaxParticles = mParticleSystem->emitterData.MaxParticles;
    emitterConstPass.EmitterIsActive = mParticleSystem->emitterData.EmitterIsActive;
    emitterConstPass.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->EmitterCB.get();
    currPassCB->CopyData(0, emitterConstPass);
}

void CBManager::UpdateParticleEmitter2CB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;

    mParticleSystem2->emitterData.StartColor = DirectX::XMFLOAT4(particle2StartColorScene6[0], particle2StartColorScene6[1], particle2StartColorScene6[2], 1.0f);
    mParticleSystem2->emitterData.EndColor = DirectX::XMFLOAT4(particle2EndColorScene6[0], particle2EndColorScene6[1], particle2EndColorScene6[2], 1.0f);
    mParticleSystem2->emitterData.StartSize = particle2StartSizeScene6;
    mParticleSystem2->emitterData.EndSize = particle2EndSizeScene6;

    emitterConstPass.Position = mParticleSystem2->emitterData.Position;
    emitterConstPass.DeltaTime = gt.DeltaTime();
    emitterConstPass.GravityForce = mParticleSystem2->emitterData.GravityForce;
    emitterConstPass.SystemID = mParticleSystem2->emitterData.SystemID;
    emitterConstPass.StartColor = mParticleSystem2->emitterData.StartColor;
    emitterConstPass.EndColor = mParticleSystem2->emitterData.EndColor;
    emitterConstPass.StartSize = mParticleSystem2->emitterData.StartSize;
    emitterConstPass.EndSize = mParticleSystem2->emitterData.EndSize;
    emitterConstPass.MaxParticles = mParticleSystem2->emitterData.MaxParticles;
    emitterConstPass.EmitterIsActive = mParticleSystem2->emitterData.EmitterIsActive;
    emitterConstPass.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->Emitter2CB.get();
    currPassCB->CopyData(0, emitterConstPass);
}

void CBManager::UpdateParticleEmitter3CB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;

    emitterConstPass.Position = mParticleSystemSmoke->emitterData.Position;
    emitterConstPass.DeltaTime = gt.DeltaTime();
    emitterConstPass.GravityForce = mParticleSystemSmoke->emitterData.GravityForce;
    emitterConstPass.SystemID = mParticleSystemSmoke->emitterData.SystemID;
    emitterConstPass.StartColor = mParticleSystemSmoke->emitterData.StartColor;
    emitterConstPass.EndColor = mParticleSystemSmoke->emitterData.EndColor;
    emitterConstPass.StartSize = mParticleSystemSmoke->emitterData.StartSize;
    emitterConstPass.EndSize = mParticleSystemSmoke->emitterData.EndSize;
    emitterConstPass.MaxParticles = mParticleSystemSmoke->emitterData.MaxParticles;
    emitterConstPass.EmitterIsActive = mParticleSystemSmoke->emitterData.EmitterIsActive;
    emitterConstPass.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->Emitter3CB.get();
    currPassCB->CopyData(0, emitterConstPass);
}

void CBManager::UpdateParticleEmitter4CB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;

    emitterConstPass.Position = mParticleSystemRain->emitterData.Position;
    emitterConstPass.DeltaTime = gt.DeltaTime();
    emitterConstPass.GravityForce = mParticleSystemRain->emitterData.GravityForce;
    emitterConstPass.SystemID = mParticleSystemRain->emitterData.SystemID;
    emitterConstPass.StartColor = mParticleSystemRain->emitterData.StartColor;
    emitterConstPass.EndColor = mParticleSystemRain->emitterData.EndColor;
    emitterConstPass.StartSize = mParticleSystemRain->emitterData.StartSize;
    emitterConstPass.EndSize = mParticleSystemRain->emitterData.EndSize;
    emitterConstPass.MaxParticles = mParticleSystemRain->emitterData.MaxParticles;
    emitterConstPass.EmitterIsActive = mParticleSystemRain->emitterData.EmitterIsActive;
    emitterConstPass.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->Emitter4CB.get();
    currPassCB->CopyData(0, emitterConstPass);
}

void CBManager::UpdatePostProcessingCB(const GameTimer& gt)
{
    PostProcessingConstants constPass;

    constPass.gGammaRatio = gammaRatioScene8;
    constPass.gTextureSize = textureSizeScene8;
    constPass.CADistortion = caDistortionScene8;
    constPass.CADirection = caDirectionScene8;
    constPass.GBIsHorizontal = gbIsHorizontalScene8;
    constPass.CAIntensity = caIntensityScene8;
    constPass.CAPadding = caPaddingScene8;
    constPass.VCenter = vCenterScene8;
    constPass.VIntensity = vIntensityScene8;
    constPass.VSmoothness = vSmoothnessScene8;
    constPass.VRoundness = vRoundnessScene8;
    constPass.NIntensity = nIntensityScene8;
    constPass.NSize = nSizeScene8;

    auto currPassCB = mCurrFrameResource->PostProcessingCB.get();
    currPassCB->CopyData(0, constPass);
}

void CBManager::UpdateNoiseCB(const GameTimer& gt)
{
    NoiseComputeConstants noiseConst;

    noiseConst.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->NoiseCB.get();
    currPassCB->CopyData(0, noiseConst);
}

void CBManager::UpdateSamplersCB(const GameTimer& gt)
{
    MoreSamplersConstants samplersConst;

    samplersConst.Flitering = filteringModeScene4;
    samplersConst.AddressMode = addressModeScene4;

    auto currPassCB = mCurrFrameResource->SamplersCB.get();
    currPassCB->CopyData(0, samplersConst);
}

void CBManager::UpdateLODCB(const GameTimer& gt)
{
    LODConstants LODConst;

    if (isUsingManualLODScene3)
        LODConst.LevelOfDetail = levelOfDetailsScene3;
    else LODConst.LevelOfDetail = 3;

    auto currPassCB = mCurrFrameResource->LODCB.get();
    currPassCB->CopyData(0, LODConst);

    if (isUsingManualLODScene3)
        LODConst.LevelOfDetail = levelOfDetailsScene3;
    else LODConst.LevelOfDetail = LODScene3;

    currPassCB = mCurrFrameResource->LODCB.get();
    currPassCB->CopyData(1, LODConst);
}

void CBManager::UpdateTessCB()
{
    TessConstants tessConst;

    if (bIsBackCullingScene11)
        tessConst.bIsBackCulling = 1;
    else tessConst.bIsBackCulling = 0;
    if (bIsDisplacementAdaptiveTessScene11)
        tessConst.DisplacementAdaptiveTess = 1;
    else tessConst.DisplacementAdaptiveTess = 0;
    if (bIsDistantAdaptiveTessScene11)
        tessConst.DistantAdaptiveTess = 1;
    else tessConst.DistantAdaptiveTess = 0;
    tessConst.TessFactor = tessFactorScene11;

    tessConst.Decals[0].Position = decalsPositionScene11[0];
    tessConst.Decals[0].DisplacementScale = decalsDisplacementScaleScene11[0];
    tessConst.Decals[0].IsActive = (UINT)decalsIsActiveScene11[0];
    tessConst.Decals[0].Scale = decalsScaleScene11[0];

    tessConst.Decals[1].Position = decalsPositionScene11[1];
    tessConst.Decals[1].DisplacementScale = decalsDisplacementScaleScene11[1];
    tessConst.Decals[1].IsActive = (UINT)decalsIsActiveScene11[1];
    tessConst.Decals[1].Scale = decalsScaleScene11[1];

    tessConst.Decals[2].Position = decalsPositionScene11[2];
    tessConst.Decals[2].DisplacementScale = decalsDisplacementScaleScene11[2];
    tessConst.Decals[2].IsActive = (UINT)decalsIsActiveScene11[2];
    tessConst.Decals[2].Scale = decalsScaleScene11[2];

    auto currPassCB = mCurrFrameResource->TessCB.get();
    currPassCB->CopyData(0, tessConst);
}

void CBManager::UpdateHeightMapCB()
{
    HeightMapConstants heightConst;

    if (isDistantAdaptiveTessScene12)
        heightConst.DistantAdaptiveTess = 1;
    else heightConst.DistantAdaptiveTess = 0;
    heightConst.TessFactor = tessFactorScene12;
    heightConst.DisplacementScale = displacementScaleScene12;

    auto currPassCB = mCurrFrameResource->HeightMapCB.get();
    currPassCB->CopyData(0, heightConst);
}

void CBManager::UpdateTerrainCB()
{
    TerrainConstants terrainConst;

    terrainConst.DisplacementScale = displacementScaleScene14;

    auto currPassCB = mCurrFrameResource->TerrainCB.get();
    currPassCB->CopyData(0, terrainConst);
}

void CBManager::UpdateScene15ObjectPosition(const GameTimer& gt)
{
    float newX = sin(gt.TotalTime() * 1.f) * 2.f;
    m_Scene15ObjectPostion = DirectX::XMFLOAT3(
        newX,
        m_Scene15ObjectPostion.y,
        m_Scene15ObjectPostion.z
    );
}

void CBManager::UpdateAtmosphereCB()
{
    AtmosphereConstants atmosphereConst;
    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();
    auto tmp1 = XMMatrixDeterminant(view);
    auto tmp2 = XMMatrixDeterminant(proj);
    DirectX::XMMATRIX invView = XMMatrixInverse(&tmp1, view);
    DirectX::XMMATRIX invProj = XMMatrixInverse(&tmp2, proj);
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto tmp3 = XMMatrixDeterminant(viewProj);
    DirectX::XMMATRIX invViewProj = XMMatrixInverse(&tmp3, viewProj);

    XMStoreFloat4x4(&atmosphereConst.InvProjectionMatrix, invProj);
    XMStoreFloat4x4(&atmosphereConst.InvViewMatrix, invView);
    XMStoreFloat4x4(&atmosphereConst.InvViewProjMatrix, invViewProj);

    atmosphereConst.DensityScaleHeight = DirectX::XMFLOAT2(8000.0f, 1200.f);
    atmosphereConst.AtmosphereRadius = 6460e3;
    atmosphereConst.EarthCenterAndRadius = DirectX::XMFLOAT4(0.f, -6360e3, 0.f, 6360e3);
    atmosphereConst.RayleiCoef = DirectX::XMFLOAT3(scene16RayleiCoef[0], scene16RayleiCoef[1], scene16RayleiCoef[2]);
    atmosphereConst.MieG = 0.8f;
    atmosphereConst.MieCoef = scene16MieCoef;
    atmosphereConst.CameraPosition = (*mCamera).GetPosition3f();

    atmosphereConst.SunColor = DirectX::XMFLOAT3(scene16SunColor[0], scene16SunColor[1], scene16SunColor[2]);
    atmosphereConst.ScaterringIntensity = scene16ScaterringIntensity;

    float posX = scene16SunPosition.x;
    float posY = scene16SunPosition.y;
    float posZ = scene16SunPosition.z;
    atmosphereConst.LightDirAndIntensity = DirectX::XMFLOAT4(posX, posY, posZ, 10.0f);

    auto currPassCB = mCurrFrameResource->AtmosphereCB.get();
    currPassCB->CopyData(0, atmosphereConst);
}

void CBManager::UpdatePaintClickCB()
{
    PaintConstants paintConst;

    paintConst.ClickPosition = CommonData::ClickPosition;
    paintConst.PaintColor = CommonData::PaintColor;

    auto currPassCB = mCurrFrameResource->PaintCB.get();
    currPassCB->CopyData(0, paintConst);
}

void CBManager::UpdateMeshRenderCB(const GameTimer& gt)
{
    DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX view = (*mCamera).GetView();
    DirectX::XMMATRIX proj = (*mCamera).GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);

    DirectX::XMStoreFloat4x4(&MeshPipeline::m_constantBufferData.World, DirectX::XMMatrixTranspose(world));
    DirectX::XMStoreFloat4x4(&MeshPipeline::m_constantBufferData.WorldView, DirectX::XMMatrixTranspose(world * view));
    DirectX::XMStoreFloat4x4(&MeshPipeline::m_constantBufferData.WorldViewProj, DirectX::XMMatrixTranspose(world * view * proj));
    MeshPipeline::m_constantBufferData.DrawMeshlets = !bIsUsingTexturingScene18;
    MeshPipeline::m_constantBufferData.Time = gt.TotalTime();
    MeshPipeline::m_constantBufferData.AnimationMode = vertexAnimationModeScene18;

    memcpy(MeshPipeline::m_cbvDataBegin, &MeshPipeline::m_constantBufferData, sizeof(MeshPipeline::m_constantBufferData));
}

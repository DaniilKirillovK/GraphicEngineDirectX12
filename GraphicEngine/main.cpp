#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <vector>
#include <string>

#include "ModelHelper.h"

#include "MathHelper.h"
#include "D3D12Engine.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Instancing.h"
#include "ParticleSystem.h"
#include "ModelLoader.h"
#include "SponzaLoader.h"
#include "ShadowMap.h"
#include "OcTree.h"
#include "QuadTree.h"
#include "TAAUtility.h"
#include "MarchingCubes.h"

#include "RootSignatureManager.h"
#include "PSOManager.h"
#include "InputLayoutShaderManager.h"
#include "UIManager.h"
#include "SamplerManager.h"
#include "CBManager.h"
#include "GeometryManager.h"

extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }


#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Config for example app
static const int APP_NUM_FRAMES_IN_FLIGHT = 2;
static const int APP_NUM_BACK_BUFFERS = 2;
static const int APP_SRV_HEAP_SIZE = 64;

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 800;

const int gNumFrameResources = 3;

struct VertexLightStage 
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

enum class RenderLayer : int
{
    Opaque = 0,
    OpaqueWireframe = 1,
    BillboardSprites = 2,
    Particles1 = 3,
    Scene2Opaque = 4,
    Scene3 = 5,
    Scene4 = 6,
    Scene5 = 7,
    Particles2 = 8,
    Particles3 = 9,
    Scene6_3 = 10,
    Scene7 = 11,
    Scene9 = 12,
    Scene10 = 13,
    Scene10DebugGeometry = 14,
    Scene11 = 15,
    Sky = 16,
    Scene3LOD = 17,
    Scene9RMDemo = 18,
    Scene12 = 19,
    Scene10MoreLight = 20,
    Scene13 = 21,
    Scene13Octree = 22,
    RainParticles = 23,
    Scene14 = 24,
    Scene15 = 25,
    Scene16 = 26,
    Scene17 = 27,
    Count
};


struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                      FenceValue;
};

ExampleDescriptorHeapAllocator mSrvHeapAllocator;
ExampleDescriptorHeapAllocator mSrvHeapAllocator2;
ExampleDescriptorHeapAllocator mSrvHeapAllocatorSponza;
ExampleDescriptorHeapAllocator mSrvHeapAllocatorShadows;
ExampleDescriptorHeapAllocator mUavHeapAllocator;

std::string ParseTime(const GameTimer &gt)
{
    int timeSec = (int)gt.TotalTime() % 1440;

    int hours = timeSec / 60;
    int minutes = timeSec % 60;

    std::string result = "";
    if (hours < 10)
    {
        result += "0";
        result += std::to_string(hours);
    }
    else result += std::to_string(hours);
    result += ":";
    if (minutes < 10)
    {
        result += "0";
        result += std::to_string(minutes);
    }
    else result += std::to_string(minutes);

    return result;
}

class Engine : public D3D12Engine
{
public:
    Engine(HINSTANCE hInstance);
    Engine(const Engine& rhs) = delete;
    Engine& operator=(const Engine& rhs) = delete;
    ~Engine();

    virtual bool Initialize()override;
    int Run() override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
    void UpdateCamera(const GameTimer& gt);
    void SetCamera2Scene3();
    void SetCamera2Scene13();
    void AnimateMaterials(const GameTimer& gt);
    void UpdateInstanceData(const GameTimer& gt);
    void UpdateInstanceDataScene13(const GameTimer& gt);
    
    void UpdateLODScene3();

    void ChangeTileObjectTiles();

    virtual void BuildShadowMaps() override;
    virtual void BuildDescriptorHeaps() override;
    void BuildShadowMapsDescriptors();
    virtual void BuildRootSignature() override;
    virtual void BuildShadersAndInputLayout() override;
    virtual void BuildSkyboxGeometry() override;
    virtual void BuildShapeGeometry() override;
    virtual void BuildScene3Geometry() override;
    virtual void BuildScene3LODGeometry() override;
    virtual void BuildScene4Geometry() override;
    virtual void BuildScene5Geometry() override;
    virtual void BuildScene6Geometry() override;
    virtual void BuildScene7Geometry() override;
    virtual void BuildScene9Geometry() override;
    virtual void BuildScene9RMDemoGeometry() override;
    virtual void BuildModelsGeometry() override;
    virtual void BuildScene10DebugGeometry() override;
    virtual void BuildScene10MoreLightGeometry() override;
    virtual void BuildScene12Geometry() override;

    virtual void BuildScene13Geometry() override;
    void BuildScene13InstanceBuffer();
    void BuildScene13OctreeGeometry();

    virtual void BuildScene14Geometry() override;
    virtual void BuildQuadTreeTerrain() override;

    virtual void BuildScene15Geometry() override;

    virtual void BuildSponzaGeometryAndTextures() override;
    virtual void BuildPSOs() override;
    virtual void BuildPostProcessingResources() override;
    virtual void CreateNoiseTexture() override;

    virtual void LoadTextures() override;
    virtual void BuildFrameResources() override;
    virtual void BuildMaterials() override;
    virtual void BuildRenderItems() override;
    virtual void BuildBillboardSpritesGeometry() override;
    virtual void UploadTextures() override;
    virtual void UploadTextures2() override;
    virtual void UploadTextures3() override;

    virtual void InitGBuffer() override;
    virtual void CreateScene3RTV() override;
    virtual void CreateScene13RTV() override;
    virtual void CreateScene15RTV() override;
    void ResizeGBuffer();

    virtual void InitInstanceBuffer() override;
    virtual void InitInstanceBufferRMDemo() override;
    virtual void InitInstanceBufferMoreLight() override;
    virtual void InitParticleSystem() override;
    virtual void InitMarchingCubesSystem() override;

    void CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc, std::string rootSigName);

    void DrawSkybox(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene3(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene3LOD(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene4(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene5(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene6(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene6Shadows(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene7(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene7Cascaded(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene7Shadows(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene9(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene9RMDemo(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene12(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene13(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene14(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawTerrainScene14(ID3D12GraphicsCommandList* cmdList);
    void DrawDebugTerrainScene14(ID3D12GraphicsCommandList* cmdList);

    void DrawRenderItemsScene15(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene15TAA(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    void DrawTerrainScene16(ID3D12GraphicsCommandList* cmdList);
    void DrawMarchingCubesScene17(ID3D12GraphicsCommandList* cmdList);

    void DrawOctreeScene13(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawDebugGeometryScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawMoreLightGeometryScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawDefferedPointSpotScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene11(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawSponzaScene(ID3D12GraphicsCommandList* cmdList);
    void DrawDebugRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawBillboardRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawScreenQuad(ID3D12GraphicsCommandList* cmdList);
    void DrawScreenQuadPostProcessing(ID3D12GraphicsCommandList* cmdList);
    void DrawScreenQuadTAA(ID3D12GraphicsCommandList* cmdList);
    void DrawScreenQuadAtmosphere(ID3D12GraphicsCommandList* cmdList);
    void DrawParticles(ParticleSystem particleSystem, RenderLayer layer);
    void DrawParticlesGPU(ParticleSystem particleSystem, RenderLayer layer, UINT CB1, UINT SRV1);
    void DrawSceneToShadowMap();
    void DrawParticlesSceneToShadowMap();
    void DrawSceneToShadowMapCascaded(int cascadedMapID);

    bool IsInCameraView(Camera camera, DirectX::XMMATRIX objectPosition, DirectX::BoundingBox itemBox);
    std::vector<DirectX::XMFLOAT4> GetFrustumCornersWorldSpace(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);
    void CalculateCascadedShadowsCameras();


private:

    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures;
    std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;

    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    std::vector<std::vector<StaticMesh>> terrainMeshesLOD;
    TerrainQuadtree m_terrainQuadTree;

    // Render items divided by PSO.
    std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;
    TAAPassConstants mTAAMainPassCB;
    PassConstantsShadows mMainPassCBShadows;
    PassConstantsShadows mShadowPassCB;
    PassConstants mReflectedPassCB;

    ParticleSystem* mParticleSystem;
    ParticleSystem* mParticleSystem2;
    ParticleSystem* mParticleSystemSmoke;
    ParticleSystem* mParticleSystemRain;

    float mTheta = 1.3f * DirectX::XM_PI;
    float mPhi = 0.4f * DirectX::XM_PI;
    float mRadius = 2.5f;

    POINT mLastMousePos;

    float mCameraSpeed = 1.0f;

    Camera* mCamera;
    Camera* mCamera2Scene3;
    Camera* mCamera2Scene13;

    Camera* mCameraFrustum3;
    Camera* mCameraFrustum2;
    Camera* mCameraFrustum1;
    Camera* mCameraFrustum0;

    Camera* mCameraShadowMap256;
    Camera* mCameraShadowMap512;
    Camera* mCameraShadowMap1024;
    Camera* mCameraShadowMap2048;

    Model Sponza;
};

bool isFirstExecution = true;




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        Engine theApp(hInstance);
        if (!theApp.Initialize())
            return 0;


        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}


int Engine::Run()
{
    MSG msg = { 0 };

    mTimer.Reset();

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {
            mTimer.Tick();

            if (!mAppPaused)
            {
                CalculateFrameStats();
                Update(mTimer);
                Draw(mTimer);
            }
            else
            {
                Sleep(100);
            }
        }
    }

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return (int)msg.wParam;
}


Engine::Engine(HINSTANCE hInstance)
    : D3D12Engine(hInstance)
{
}

Engine::~Engine()
{
}

bool Engine::Initialize()
{
    mCamera = new Camera();
    mCameraFrustum0 = new Camera();
    mCameraFrustum1 = new Camera();
    mCameraFrustum2 = new Camera();
    mCameraFrustum3 = new Camera();

    mCameraShadowMap256 = new Camera();
    mCameraShadowMap512 = new Camera();
    mCameraShadowMap1024 = new Camera();
    mCameraShadowMap2048 = new Camera();

    mCamera2Scene3 = new Camera();
    mCamera2Scene13 = new Camera();

    mCamera->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    mCameraFrustum0->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 30.0f, 1000.0f);
    mCameraFrustum1->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 20.0f, 30.0f);
    mCameraFrustum2->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 10.0f, 20.0f);
    mCameraFrustum3->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 10.0f);

    mCameraShadowMap256->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    mCameraShadowMap512->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    mCameraShadowMap1024->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    mCameraShadowMap2048->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    mCamera2Scene3->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    mCamera2Scene13->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    SetCamera2Scene3();
    SetCamera2Scene13();
    DirectX::BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera->GetProj());

    if (!D3D12Engine::Initialize())
        return false;

    return true;
}

void Engine::OnResize()
{
    //ResizeGBuffer();
    D3D12Engine::OnResize();
}

void Engine::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);

    // Cycle through the circular frame resource array.
    CBManager::mCurrFrameResourceIndex = (CBManager::mCurrFrameResourceIndex + 1) % gNumFrameResources;
    CBManager::mCurrFrameResource = CBManager::mFrameResources[CBManager::mCurrFrameResourceIndex].get();

    CBManager::CBManager::mCurrFrameResource = CBManager::mCurrFrameResource;
    CBManager::mCamera = mCamera;
    CBManager::mCamera2Scene13 = mCamera2Scene13;
    CBManager::mCamera2Scene3 = mCamera2Scene3;
    CBManager::mCameraFrustum0 = mCameraFrustum0;
    CBManager::mCameraFrustum1 = mCameraFrustum1;
    CBManager::mCameraFrustum2 = mCameraFrustum2;
    CBManager::mCameraFrustum3 = mCameraFrustum3;
    CBManager::mCameraShadowMap256 = mCameraShadowMap256;
    CBManager::mCameraShadowMap512 = mCameraShadowMap512;
    CBManager::mCameraShadowMap1024 = mCameraShadowMap1024;
    CBManager::mCameraShadowMap2048 = mCameraShadowMap2048;
    CBManager::CBManager::mCurrFrameResource = CBManager::mCurrFrameResource;
    CBManager::mParticleSystem = mParticleSystem;
    CBManager::mParticleSystem2 = mParticleSystem2;
    CBManager::mParticleSystemRain = mParticleSystemRain;
    CBManager::mParticleSystemSmoke = mParticleSystemSmoke;

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (CBManager::mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < CBManager::mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(CBManager::mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    mLightRotationAngle = directionalLightPositionScene7;

    DirectX::XMMATRIX R = DirectX::XMMatrixRotationY(mLightRotationAngle);
    for (int i = 0; i < 3; ++i)
    {
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&mBaseLightDirections[i]);
        lightDir = XMVector3TransformNormal(lightDir, R);
        XMStoreFloat3(&mRotatedLightDirections[i], lightDir);
    }
    if (isVerticalLightScene7)
    {
        DirectX::XMVECTOR lightDir = XMLoadFloat3(&mBaseLightDirections[1]);
        XMStoreFloat3(&mRotatedLightDirections[0], lightDir);
    }

    if (isAnimateMaterialScene4)
        AnimateMaterials(gt);

    if (activeSceneID == 3 && isUsingInstancingScene3 && isFrustumCullingScene3)
        UpdateInstanceData(gt);

    CBManager::UpdateObjectCBs(gt);
    CBManager::UpdateMaterialCBs(gt);

    if (activeSceneID != 10)
        CBManager::UpdateMainPassCB(gt);

    if (activeSceneID == 3)
    {
        CBManager::UpdateMainPassCBScene3Camera2(gt);
        CBManager::UpdateLODCB(gt);
    }
    if (activeSceneID == 4)
    {
        CBManager::UpdateSamplersCB(gt);
    }
    if (activeSceneID == 6)
    {
        if (activeParticleSystemScene6 == 1)
            CBManager::UpdateParticleEmitterCB(gt);
        else if (activeParticleSystemScene6 == 2)
            CBManager::UpdateParticleEmitter2CB(gt);
        else if (activeParticleSystemScene6 == 3)
        {
            CBManager::UpdateParticleEmitter3CB(gt);
            CBManager::UpdateParticleEmitter4CB(gt);
            CBManager::UpdateShadowPassCBParticles(gt);
            CBManager::UpdateShadowTransform(gt);
            CBManager::UpdateMainPassCBShadows(gt);
            CBManager::UpdateMainPassCBParticles(gt);
        }
    }
    if (activeSceneID == 7)
    {
        CBManager::UpdateMainPassCBShadows(gt);
        CBManager::UpdateShadowPassCB(gt);
        CBManager::UpdateShadowTransform(gt);
        if (isUsingCascadedShadowsScene7)
        {
            CalculateCascadedShadowsCameras();
            CBManager::UpdateShadowPassCBCascaded(gt);
        }
    }
    if (activeSceneID == 8)
    {
        CBManager::UpdatePostProcessingCB(gt);
        if (isActiveNoiseScene8)
        {
            CBManager::UpdateNoiseCB(gt);
        }
    }
    if (activeSceneID == 10)
    {
        if (isMoreLightActiveScene10)
        {
            CBManager::UpdateMainPassCBMoreLightScene10(gt);
            CBManager::UpdateLightObjectCBMoreLight(gt);
        }
        else CBManager::UpdateMainPassCBScene10(gt);
        CBManager::UpdateLightObjectCBs(gt);
    }
    if (activeSceneID == 11)
    {
        CBManager::UpdateTessCB();
    }
    if (activeSceneID == 12)
    {
        CBManager::UpdateHeightMapCB();
    }
    if (activeSceneID == 13)
    {
        CBManager::UpdateMainPassCBScene13Camera2(gt);
        UpdateInstanceDataScene13(gt);
    }
    if (activeSceneID == 14)
    {
        CBManager::UpdateTerrainCB();
    }
    if (activeSceneID == 15)
    {
        if (isMovingObjectScene15)
            CBManager::UpdateScene15ObjectPosition(gt);

        if (selectedRenderModeScene15 == 1)
        {
            CBManager::UpdateMainPassCBScene15(gt);
            CBManager::UpdateTAAObjectCBs(gt);
        }
    }
    if (activeSceneID == 16)
    {
        CBManager::UpdateAtmosphereCB();
    }
    timeScene2 = ParseTime(gt);
}

void WaitForGPU(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence, UINT64& fenceValue)
{
    const UINT64 currentFenceValue = fenceValue;
    commandQueue->Signal(fence, currentFenceValue);
    fenceValue++;

    if (fence->GetCompletedValue() < currentFenceValue)
    {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fence->SetEventOnCompletion(currentFenceValue, eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void Engine::Draw(const GameTimer& gt)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    WaitForGPU(mCommandQueue.Get(), mFence.Get(), mCurrentFence);
    ThrowIfFailed(mDirectCmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    if (activeSceneID != 7)
    {
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs["deferredLighting"].Get()));
    }
    else
    {
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs["shadowForwardPSO"].Get()));
    }

    D3D12_VIEWPORT viewports[] = { mScreenViewport, mScreenViewport2, mScreenViewport3, mScreenViewport4, mScreenViewportFull };
    D3D12_RECT rects[] = { mScissorRect, mScissorRect2, mScissorRect3, mScissorRect4, mScissorRectFull };

    // Clear the back buffer and depth buffer.
    auto barrier1Clear = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    auto barrier2Clear = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    auto barrier3Clear = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    auto barrier4Clear = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    auto barrier5Clear = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    CD3DX12_RESOURCE_BARRIER barriersClearOpen[] = { barrier1Clear, barrier2Clear, barrier3Clear, barrier4Clear, barrier5Clear };
    mCommandList->ResourceBarrier(5, barriersClearOpen);
    if (activeSceneID == 3)
    {
        auto barrierClear = CD3DX12_RESOURCE_BARRIER::Transition(Scene3RenderTargetBuffer(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &barrierClear);
    }
    if (activeSceneID == 13)
    {
        auto barrierClear = CD3DX12_RESOURCE_BARRIER::Transition(Scene13RenderTargetBuffer(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &barrierClear);
    }

    
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
    if (activeSceneID != 7)
    {
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 6, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 8, mRtvDescriptorSize), DirectX::Colors::LightSkyBlue, 0, nullptr);
    }

    auto barrier1ClearEnd = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    auto barrier2ClearEnd = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto barrier3ClearEnd = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto barrier4ClearEnd = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto barrier5ClearEnd = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    CD3DX12_RESOURCE_BARRIER barriersClearClose[] = { barrier1ClearEnd, barrier2ClearEnd, barrier3ClearEnd, barrier4ClearEnd, barrier5ClearEnd };
    mCommandList->ResourceBarrier(5, barriersClearClose);

    if (activeSceneID == 13)
    {
        auto barrierClear = CD3DX12_RESOURCE_BARRIER::Transition(Scene13RenderTargetBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        mCommandList->ResourceBarrier(1, &barrierClear);
    }


    if (isFirstExecution)
    {
        auto depthBufferTransition = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        CD3DX12_RESOURCE_BARRIER barriersFirstExec[1] = { depthBufferTransition };
        mCommandList->ResourceBarrier(1, barriersFirstExec);

        isFirstExecution = false;
    }

        mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    if (activeSceneID == 3)
        mCommandList->ClearDepthStencilView(DepthStencilViewScene3(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    if (activeSceneID == 13)
        mCommandList->ClearDepthStencilView(DepthStencilViewScene13(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = 
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize),
    };

    D3D12_CPU_DESCRIPTOR_HANDLE postProcessingRTV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mRtvDescriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvsForward[] = 
    {
        CurrentBackBufferView(),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize),
    };

    auto dsv = DepthStencilView();

    ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap.Get() };
    auto passCB = CBManager::mCurrFrameResource->PassCB->Resource();

    if (isFirstExecution)
    {
        isFirstExecution = false;
    }

    if (activeSceneID == 1)
    {
        // Draw Wireframe (debug)
        if (isDebug)
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDebug"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

            // Indicate a state transition on the resource usage.
            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular};
            mCommandList->ResourceBarrier(5, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["debug"].Get());


            DrawDebugRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::OpaqueWireframe]);
        }

        // Draw billboards
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureBillboard"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["billboardSprites"].Get());


            DrawBillboardRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::BillboardSprites]);
        }

        // Draw Opaque
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignature"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (isSolid && !isPixelated) mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());
            else if (isPixelated) mCommandList->SetPipelineState(mPSOs["opaquePixel"].Get());
            else mCommandList->SetPipelineState(mPSOs["opaqueWireframe"].Get());


            DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);


            // Indicate a state transition on the resource usage.
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            CD3DX12_RESOURCE_BARRIER barriersClose[] = {  barrier2, barrier3, barrier4, barrier5, barrier6 };
            mCommandList->ResourceBarrier(5, barriersClose);
        }
    }

    else if (activeSceneID == 2)
    {
        // Draw calls
        // Draw Opaque
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignature"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            // Indicate a state transition on the resource usage.
            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto depth = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth };
            mCommandList->ResourceBarrier(6, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);
            
            mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());

            DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene2Opaque]);


            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier1, barrier2, barrier3, barrier4, barrier5, barrier6 };
            mCommandList->ResourceBarrier(6, barriersClose);
        }
    }

    else if (activeSceneID == 3)
    {
        // Draw calls
        // Camera 2 Call
        if (isDisplayingFrustumCullingInfoScene3 && isUsingInstancingScene3 && !isFrustumCullingScene3)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = Scene3RenderTargetBufferView();
            auto passCBCamera2 = CBManager::mCurrFrameResource->PassCBScene3Camera2->Resource();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCBCamera2->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto rtvBuffer = CD3DX12_RESOURCE_BARRIER::Transition(Scene3RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &rtvBuffer);

            auto dsvCamera2 = DepthStencilViewScene3();
            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsvCamera2);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(Scene3RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else if (isUsingInstancingScene3 && isFrustumCullingScene3 && isDisplayingFrustumCullingInfoScene3)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = Scene3RenderTargetBufferView();
            auto passCBCamera2 = CBManager::mCurrFrameResource->PassCBScene3Camera2->Resource();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForwardFrustumCulling"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCBCamera2->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto rtvBuffer = CD3DX12_RESOURCE_BARRIER::Transition(Scene3RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &rtvBuffer);

            auto dsvCamera2 = DepthStencilViewScene3();
            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsvCamera2);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefaultFrustumCulling"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(Scene3RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier1);
        }

        // Main Camera Call
        if (!isFrustumCullingScene3)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else if (isUsingInstancingScene3)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForwardFrustumCulling"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefaultFrustumCulling"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }

        // LOD
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            UINT LODCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LODConstants));
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            D3D12_GPU_VIRTUAL_ADDRESS lodCBAddress = LODCB->GetGPUVirtualAddress() + LODCBByteSize;
            mCommandList->SetGraphicsRootConstantBufferView(5, lodCBAddress);

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["forwardDefault"].Get());

            UpdateLODScene3();
            DrawRenderItemsScene3LOD(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene3LOD]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
    }
    else if (activeSceneID == 4)
    {
        // Draw calls
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureMoreSamplers"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto samplersCB = CBManager::mCurrFrameResource->SamplersCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, samplersCB->GetGPUVirtualAddress());

            // Indicate a state transition on the resource usage.
            auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            D3D12_RESOURCE_BARRIER barriers[] = { albedo, position, normal, specular };
            mCommandList->ResourceBarrier(4, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["moreSamplers"].Get());

            DrawRenderItemsScene4(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene4]);

            // Indicate a state transition on the resource usage.
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier2, barrier3, barrier4, barrier5, barrier6 };
            mCommandList->ResourceBarrier(5, barriersClose);
        }
    }
    else if (activeSceneID == 5)
    {
        //Draw calls
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignature"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (isSolidScene5) mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());
            else mCommandList->SetPipelineState(mPSOs["opaqueWireframe"].Get());

            DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene5]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier1, barrier2, barrier3, barrier4, barrier5, barrier6 };
            mCommandList->ResourceBarrier(6, barriersClose);
        }
    }
    else if (activeSceneID == 6)
    {
        // Particles
        if (particles1IsActive && activeParticleSystemScene6 == 1)
        {
            // Compute pass
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleBuffers[1 - currParticleReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->EmitterCB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleBuffers[1 - currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleBuffers[currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["computeParticles"].Get());

                UINT threadGroupCount = 8;
                mCommandList->Dispatch(threadGroupCount, 1, 1);

                CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleBuffers[1 - currParticleReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
                mCommandList->ResourceBarrier(1, &computeBarrier2);

                currParticleReadBuffer = 1 - currParticleReadBuffer;
            }

            // Draw calls
            // Draw particles
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureParticlesRender"].Get());

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(currParticleReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["renderParticles"].Get());

                DrawParticles(*mParticleSystem, RenderLayer::Particles1);
            }
        }
        else if (particles2IsActive && activeParticleSystemScene6 == 2)
        {
            // Compute pass
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particle2Buffers[1 - currParticle2ReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter2CB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particle2Buffers[1 - currParticle2ReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particle2Buffers[currParticle2ReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["computeParticles"].Get());

                UINT threadGroupCount = 8;
                mCommandList->Dispatch(threadGroupCount, 1, 1);

                CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particle2Buffers[1 - currParticle2ReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
                mCommandList->ResourceBarrier(1, &computeBarrier2);

                currParticle2ReadBuffer = 1 - currParticle2ReadBuffer;
            }

            // Draw calls
            // Draw particles
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureParticlesRender"].Get());

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(5 + currParticle2ReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["renderParticles"].Get());

                DrawParticles(*mParticleSystem2, RenderLayer::Particles2);
            }
        }
        else if (activeParticleSystemScene6 == 3)
        {
            //// Compute args pass
            //{
            //    ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
            //    mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

            //    CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
            //        particleSmokeArgsBuffer.Get(),
            //        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
            //        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            //    mCommandList->ResourceBarrier(1, &computeBarrier1);

            //    mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureParticlesArgsCompute"].Get());

            //    mCommandList->SetComputeRootUnorderedAccessView(0, particleSmokeArgsBuffer->GetGPUVirtualAddress());

            //    mCommandList->SetPipelineState(mPSOs["computeParticlesArgs"].Get());

            //    UINT threadGroupCount = 1;
            //    mCommandList->Dispatch(threadGroupCount, 1, 1);

            //    CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
            //        particleSmokeArgsBuffer.Get(),
            //        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            //        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
            //    mCommandList->ResourceBarrier(1, &computeBarrier2);
            //}

            // Compute pass Smoke
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleSmokeBuffers[1 - currParticleSmokeReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter3CB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleSmokeBuffers[1 - currParticleSmokeReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleSmokeBuffers[currParticleSmokeReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["computeParticles"].Get());

                UINT threadGroupCount = 8;
                mCommandList->Dispatch(threadGroupCount, 1, 1);

                CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleSmokeBuffers[1 - currParticleSmokeReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
                mCommandList->ResourceBarrier(1, &computeBarrier2);

                currParticleSmokeReadBuffer = 1 - currParticleSmokeReadBuffer;
            }

            // Compute pass Rain
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleRainBuffers[1 - currParticleRainReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter4CB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleRainBuffers[1 - currParticleRainReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleRainBuffers[currParticleRainReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["computeParticlesRain"].Get());

                UINT threadGroupCount = 8;
                mCommandList->Dispatch(threadGroupCount, 1, 1);

                CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleRainBuffers[1 - currParticleRainReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
                mCommandList->ResourceBarrier(1, &computeBarrier2);

                currParticleRainReadBuffer = 1 - currParticleRainReadBuffer;
            }

            // Shadow Pass
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureShadows"].Get());

                auto passCB = CBManager::mCurrFrameResource->ShadowPassCBParticles->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawParticlesSceneToShadowMap();
            }

            // Draw calls
            // Draw objects
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

                 mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureShadowsParticlesForward"].Get());

                auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
                mCommandList->ResourceBarrier(1, &barrier1);

                mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

                mCommandList->SetPipelineState(mPSOs["shadowForwardParticlesPSO"].Get());

                {
                    passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
                    mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
                    DrawRenderItemsScene6Shadows(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene6_3]);
                }
            }

            // Draw particles Smoke
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureParticlesRender"].Get());

                auto passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(10 + currParticleSmokeReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                if (lightingIDScene6 == 2)
                    mCommandList->SetPipelineState(mPSOs["renderParticlesForward"].Get());
                else mCommandList->SetPipelineState(mPSOs["renderParticlesForwardVertexLighting"].Get());

                DrawParticles(*mParticleSystemSmoke, RenderLayer::Particles3);
                //DrawParticlesGPU(*mParticleSystemSmoke, RenderLayer::Particles3);
            }

            // Draw particles Rain
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureParticlesRain"].Get());

                auto passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(15 + currParticleRainReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["particlesRain"].Get());

                DrawParticles(*mParticleSystemRain, RenderLayer::RainParticles);

                // Indicate a state transition on the resource usage.
                auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
                mCommandList->ResourceBarrier(1, &barrier2);
            }
        }
    }

    else if (activeSceneID == 7)
    {
        ID3D12DescriptorHeap* descriptorHeapsShadowPass[] = { mSponzaSrvHeap.Get() };
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsShadowPass), descriptorHeapsShadowPass);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureShadows"].Get());

        UINT shadowCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstantsShadows));
        auto shadowCB = CBManager::mCurrFrameResource->ShadowPassCBCascaded->Resource();

        auto passCB = CBManager::mCurrFrameResource->ShadowPassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

        if (isUsingCascadedShadowsScene7)
        {
            /*D3D12_GPU_VIRTUAL_ADDRESS shadowCBAddress = shadowCB->GetGPUVirtualAddress() + 0 * shadowCBByteSize;
            mCommandList->SetGraphicsRootConstantBufferView(2, shadowCBAddress);*/
            DrawSceneToShadowMapCascaded(0);

            /*shadowCBAddress = shadowCB->GetGPUVirtualAddress() + 1 * shadowCBByteSize;
            mCommandList->SetGraphicsRootConstantBufferView(2, shadowCBAddress);*/
            DrawSceneToShadowMapCascaded(1);

            /*shadowCBAddress = shadowCB->GetGPUVirtualAddress() + 2 * shadowCBByteSize;
            mCommandList->SetGraphicsRootConstantBufferView(2, shadowCBAddress);*/
            DrawSceneToShadowMapCascaded(2);

            /*shadowCBAddress = shadowCB->GetGPUVirtualAddress() + 3 * shadowCBByteSize;
            mCommandList->SetGraphicsRootConstantBufferView(2, shadowCBAddress);*/
            DrawSceneToShadowMapCascaded(3);
        }
        else
        {
            /*auto passCB = CBManager::mCurrFrameResource->ShadowPassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());*/
            DrawSceneToShadowMap();
        }

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureShadowsForward"].Get());

        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &barrier1);

        mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

        CD3DX12_GPU_DESCRIPTOR_HANDLE shadowTextures(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        shadowTextures.Offset(28, mCbvSrvDescriptorSize);
        mCommandList->SetGraphicsRootDescriptorTable(5, shadowTextures);

        mCommandList->SetPipelineState(mPSOs["shadowForwardPSO"].Get());

        if (isUsingCascadedShadowsScene7)
        {
            passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            DrawRenderItemsScene7Cascaded(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene7]);
        }
        else 
        {
            passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            DrawRenderItemsScene7(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene7]);
        }

        // Indicate a state transition on the resource usage.
        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier2);
    }


    else if (activeSceneID == 8)
    {
        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureRT"].Get());

        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

        // Indicate a state transition on the resource usage.
        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto depth = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth };
        mCommandList->ResourceBarrier(6, barriers);

        mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetPipelineState(mPSOs["defferedRT"].Get());

        ID3D12DescriptorHeap* descriptorHeapSponza[] = { mSponzaSrvHeap.Get() };
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeapSponza), descriptorHeapSponza);
        DrawSponzaScene(mCommandList.Get());

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier1, barrier2, barrier3, barrier4, barrier5, barrier6 };
        mCommandList->ResourceBarrier(6, barriersClose);
    }

    else if (activeSceneID == 9)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (!isRoughnessMetallicDemoScene9)
        {
            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignaturePBR"].Get());

            if (PBRShaderScene9 == 0)
                mCommandList->SetPipelineState(mPSOs["StandartPBRPSO"].Get());
            else if (PBRShaderScene9 == 1)
                mCommandList->SetPipelineState(mPSOs["PBRPSO"].Get());
            else if (PBRShaderScene9 == 2)
                mCommandList->SetPipelineState(mPSOs["IBLPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(48, mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(4, tex);

            DrawRenderItemsScene9(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene9]);
        }
        else
        {
            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureRMDemo"].Get());

            mCommandList->SetPipelineState(mPSOs["RMDemoPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(48, mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(4, tex);
            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceBuffer(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceBuffer.Offset(53, mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(5, instanceBuffer);

            DrawRenderItemsScene9RMDemo(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene9RMDemo]);
        }

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    else if (activeSceneID == 10)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureRT"].Get());

        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (selectedRenderTechScene10 != 0)
        {
            // Indicate a state transition on the resource usage.
            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular };
            mCommandList->ResourceBarrier(5, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            if (selectedRenderTechScene10 == 1 && isMoreLightActiveScene10)
                mCommandList->SetPipelineState(mPSOs["defferedRT"].Get());
            else if (isUsingPositionBufferScene10 || selectedRenderTechScene10 == 2)
                mCommandList->SetPipelineState(mPSOs["defferedRT"].Get());
            else mCommandList->SetPipelineState(mPSOs["defferedRTNoPos"].Get());

            ID3D12DescriptorHeap* descriptorHeapSponza[] = { mSponzaSrvHeap.Get() };
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeapSponza), descriptorHeapSponza);
            DrawSponzaScene(mCommandList.Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
            DrawRenderItemsScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier1, barrier2, barrier3, barrier4, barrier5, barrier6 };
            mCommandList->ResourceBarrier(6, barriersClose);
        }
        else
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (!isMoreLightActiveScene10)
                mCommandList->SetPipelineState(mPSOs["forwardRT"].Get());
            else mCommandList->SetPipelineState(mPSOs["forwardRTMoreLight"].Get());

            ID3D12DescriptorHeap* descriptorHeapSponza[] = { mSponzaSrvHeap.Get() };
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeapSponza), descriptorHeapSponza);
            DrawSponzaScene(mCommandList.Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
            DrawRenderItemsScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }

        // Light accumulation
        if (selectedRenderTechScene10 == 2)
        {
            // Point & Spot lights
            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDefferedPointSpotDirectional"].Get());

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["defferedPointSpot"].Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawDefferedPointSpotScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10DebugGeometry]);

            // More Light
            if (isMoreLightActiveScene10)
            {
                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureMoreLight"].Get());

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, nullptr);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["defferedPointSpotMoreLight"].Get());

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE instanceBuffer(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                instanceBuffer.Offset(56, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(3, instanceBuffer);

                DrawMoreLightGeometryScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10MoreLight]);
            }

            // Directional & ambient light
            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureLight"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["defferedDirectional"].Get());
            DrawScreenQuad(mCommandList.Get());

            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            CD3DX12_RESOURCE_BARRIER barriers[] = { barrier1, barrier2 };
            mCommandList->ResourceBarrier(2, barriers);
        }
    }

    else if (activeSceneID == 11)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureTess"].Get());

        mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
        auto tessCB = CBManager::mCurrFrameResource->TessCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(5, tessCB->GetGPUVirtualAddress());

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        // Indicate a state transition on the resource usage.
        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto position = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto albedo = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto normal = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto specular = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        auto depth = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth };
        mCommandList->ResourceBarrier(6, barriers);

        mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

        if (!isWireframeScene11)
            mCommandList->SetPipelineState(mPSOs["decalsTess"].Get());
        else mCommandList->SetPipelineState(mPSOs["decalsTessWireframe"].Get());

        DrawRenderItemsScene11(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene11]);

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferAlbedo.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier3 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferPosition.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier4 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferNormal.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier5 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferSpecular.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        auto barrier6 = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        CD3DX12_RESOURCE_BARRIER barriersClose[] = { barrier1, barrier2, barrier3, barrier4, barrier5, barrier6 };
        mCommandList->ResourceBarrier(6, barriersClose);
    }

    else if (activeSceneID == 12)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureHeightMap"].Get());

        if (isWireframeScene12)
            mCommandList->SetPipelineState(mPSOs["heightMapWireframePSO"].Get());
        else mCommandList->SetPipelineState(mPSOs["heightMapSolidPSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
        auto heightCB = CBManager::mCurrFrameResource->HeightMapCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(4, heightCB->GetGPUVirtualAddress());

        DrawRenderItemsScene12(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene12]);

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    else if (activeSceneID == 13)
    {
        // Main camera Draw call
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureObjectsScene13"].Get());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            auto instanceCB = CBManager::mCurrFrameResource->InstancingScene13CB->Resource();
            mCommandList->SetGraphicsRootShaderResourceView(4, instanceCB->GetGPUVirtualAddress());

            mCommandList->SetPipelineState(mPSOs["Scene13ObjectsPSO"].Get());

            DrawRenderItemsScene13(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene13]);


            // Draw Octree
            if (isDebugOctreeScene13)
            {
                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureOctreeScene13"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["Scene13OctreePSO"].Get());

                DrawOctreeScene13(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene13Octree]);
            }

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }

        // Second camera Draw call
        if (isActiveOctreeCullingInfoScene13)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = Scene13RenderTargetBufferView();
            auto passCBCamera2 = CBManager::mCurrFrameResource->PassCBScene13Camera2->Resource();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureObjectsScene13"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(2, passCBCamera2->GetGPUVirtualAddress());
            auto instanceCB = CBManager::mCurrFrameResource->InstancingScene13CB->Resource();
            mCommandList->SetGraphicsRootShaderResourceView(4, instanceCB->GetGPUVirtualAddress());

            auto rtvBuffer = CD3DX12_RESOURCE_BARRIER::Transition(Scene13RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &rtvBuffer);

            auto dsvCamera2 = DepthStencilViewScene13();
            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsvCamera2);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["Scene13ObjectsPSO"].Get());

            DrawRenderItemsScene13(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene13]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(Scene13RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
    }

    else if (activeSceneID == 14)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureTerrain"].Get());

        if (!isWireframeScene14)
            mCommandList->SetPipelineState(mPSOs["terrainPSO"].Get());
        else mCommandList->SetPipelineState(mPSOs["terrainWireframePSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
        auto terrainCB = CBManager::mCurrFrameResource->TerrainCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(4, terrainCB->GetGPUVirtualAddress());

        if (isWireframeScene14)
            DrawDebugTerrainScene14(mCommandList.Get());
        else DrawTerrainScene14(mCommandList.Get());

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    else if (activeSceneID == 15)
    {
        if (selectedRenderModeScene15 == 0)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->ClearRenderTargetView(rtvs2, DirectX::Colors::White, 0, nullptr);

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignaturePBR"].Get());

            mCommandList->SetPipelineState(mPSOs["PBRPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawRenderItemsScene15(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene15]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else
        {
            {
                CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
                CD3DX12_CPU_DESCRIPTOR_HANDLE velocityTextureRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 11, mRtvDescriptorSize);
                handle.Offset(9 + TAAUtility::m_CurrentRTV, mRtvDescriptorSize);

                mCommandList->ClearRenderTargetView(handle, DirectX::Colors::White, 0, nullptr);
                mCommandList->ClearRenderTargetView(velocityTextureRTVHandle, DirectX::Colors::Black, 0, nullptr);

                TAAUtility::m_CurrentRTV = (TAAUtility::m_CurrentRTV + 1) % 2;

                D3D12_CPU_DESCRIPTOR_HANDLE rtvsTaa[] =
                {
                    handle,
                    velocityTextureRTVHandle
                };

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->OMSetRenderTargets(2, rtvsTaa, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureTAA"].Get());

                mCommandList->SetPipelineState(mPSOs["TAAPSO"].Get());

                passCB = CBManager::mCurrFrameResource->TAAPassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawRenderItemsScene15TAA(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene15]);

                /*mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureSkyBox"].Get());

                mCommandList->OMSetRenderTargets(1, &handle, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["skyboxPSO"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawSkybox(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);*/
            }

            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureTAASecondPass"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                mCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["TAASecondPassPSO"].Get());

                DrawScreenQuadTAA(mCommandList.Get());
            }
        }
    }

    else if (activeSceneID == 16)
    {
        // Geometry Pass
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureTerrain"].Get());

            mCommandList->SetPipelineState(mPSOs["terrainPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            auto terrainCB = CBManager::mCurrFrameResource->TerrainCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(4, terrainCB->GetGPUVirtualAddress());

            DrawTerrainScene16(mCommandList.Get());

            /*mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureSkyBox"].Get());

            mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["skyboxPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawSkybox(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);*/
        }

        // Atmosphere pass
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

            // Indicate a state transition on the resource usage.
            D3D12_RESOURCE_BARRIER depthBarrier = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &depthBarrier);

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureAtmosphere"].Get());

            passCB = CBManager::mCurrFrameResource->AtmosphereCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());

            CD3DX12_GPU_DESCRIPTOR_HANDLE depthSrv(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            depthSrv.Offset(15, mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(1, depthSrv);

            mCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["AtmospherePSO"].Get());

            DrawScreenQuadAtmosphere(mCommandList.Get());

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);

            auto depthOpen = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            D3D12_RESOURCE_BARRIER barrierOpen[] = { depthOpen };
            mCommandList->ResourceBarrier(1, barrierOpen);
        }
    }

    else if (activeSceneID == 17)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureCubeMarching"].Get());

        mCommandList->SetPipelineState(mPSOs["CubeMarchingPSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        DrawMarchingCubesScene17(mCommandList.Get());

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }
    

    // Draw screen quad
    if ((activeSceneID <= 2 || activeSceneID >= 4) && !(activeSceneID == 10 && selectedRenderTechScene10 == 0) && activeSceneID != 7 && !(activeSceneID == 6 && activeParticleSystemScene6 == 3)
        && !(activeSceneID == 10 && selectedRenderTechScene10 == 2) && activeSceneID != 9 && activeSceneID != 12
        && activeSceneID != 13 && activeSceneID != 14 && activeSceneID != 15 && activeSceneID != 16 && activeSceneID != 17)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2;
        if (activeSceneID == 8)
        {
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(postProcessingBuffer.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &barrier1);
            rtvs2 = postProcessingRTV;
        }
        else rtvs2 = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureLight"].Get());

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (activeSceneID == 10 && isMoreLightActiveScene10)
            mCommandList->SetPipelineState(mPSOs["defferedLightingMoreLight"].Get());
        else if (!(activeSceneID == 10 && !isUsingPositionBufferScene10))
            mCommandList->SetPipelineState(mPSOs["deferredLighting"].Get());
        else mCommandList->SetPipelineState(mPSOs["deferredLightingNoPos"].Get());

        DrawScreenQuad(mCommandList.Get());

        if (activeSceneID == 8)
        {
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(postProcessingBuffer.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier2);
        }

        auto depthOpen = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        D3D12_RESOURCE_BARRIER barrierOpen[] = { depthOpen };
        mCommandList->ResourceBarrier(1, barrierOpen);
    }

    // Draw Debug Geometry Scene 10
    if (activeSceneID == 10 && isDebugLayerActiveScene10)
    {
        auto depthOpen = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        D3D12_RESOURCE_BARRIER barrierOpen[] = { depthOpen };
        mCommandList->ResourceBarrier(1, barrierOpen);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureDebugGeometry"].Get());

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->SetPipelineState(mPSOs["debugGeometry"].Get());

        DrawDebugGeometryScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10DebugGeometry]);
    }

    // SkyBox
    if (!(activeSceneID == 15) && activeSceneID != 16)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureSkyBox"].Get());

        mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetPipelineState(mPSOs["skyboxPSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

        DrawSkybox(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    if (activeSceneID == 8)
    {
        // Noise compute
        if (isActiveNoiseScene8)
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetComputeRootSignature(mRootSignatures["mRootSignatureComputeNoise"].Get());

            CD3DX12_GPU_DESCRIPTOR_HANDLE uav(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            uav.Offset(21, mCbvSrvDescriptorSize);
            auto noiseCB = CBManager::mCurrFrameResource->NoiseCB->Resource();

            mCommandList->SetComputeRootDescriptorTable(0, uav);
            mCommandList->SetComputeRootConstantBufferView(1, noiseCB->GetGPUVirtualAddress());

            mCommandList->SetPipelineState(mPSOs["computeNoise"].Get());

            UINT threadGroupCountX = WINDOW_WIDTH / 8.0f;
            UINT threadGroupCountY = WINDOW_HEIGHT / 8.0f;
            mCommandList->Dispatch(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignaturePostProcessing"].Get());

        auto postProcessingPassCB = CBManager::mCurrFrameResource->PostProcessingCB->Resource();

        mCommandList->SetGraphicsRootConstantBufferView(1, postProcessingPassCB->GetGPUVirtualAddress());

        CD3DX12_GPU_DESCRIPTOR_HANDLE srv(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        srv.Offset(22, mCbvSrvDescriptorSize);

        mCommandList->SetGraphicsRootDescriptorTable(2, srv);

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (isActiveNormalScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_Default"].Get());
        else if (isActiveGCScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_GC"].Get());
        else if (isActiveGBScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_GB"].Get());
        else if (isActiveCAScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_CA"].Get());
        else if (isActiveVigScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_Vig"].Get());
        else if (isActiveNoiseScene8)
            mCommandList->SetPipelineState(mPSOs["postProcessing_Noise"].Get());

        DrawScreenQuadPostProcessing(mCommandList.Get());
    }

    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    auto barrierLast1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mCommandList->ResourceBarrier(1, &barrierLast1);

    // Imgui
    UIManager::RenderUI(mCommandList, GeometryManager::mAllRitems, mSrvHeap, mCbvSrvDescriptorSize, GeometryManager::mMaterials);

    auto barrierLast2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    mCommandList->ResourceBarrier(1, &barrierLast2);

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    HRESULT deviceRemovedReason = md3dDevice->GetDeviceRemovedReason();
    if (FAILED(deviceRemovedReason))
    {
        int i = 0;
    }

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    CBManager::mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Engine::OnMouseDown(WPARAM btnState, int x, int y)
{
    if (x <= 1024 
        && (x <= 960 && y <= 600))
    {
        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }

    SetCapture(mhMainWnd);
}

void Engine::OnMouseUp(WPARAM btnState, int x, int y)
{
    if (x <= 1024
        && (x <= 960 && y <= 600))
        ReleaseCapture();
}

void Engine::OnMouseMove(WPARAM btnState, int x, int y)
{
    if (x <= 1024
        && (x <= 960 && y <= 600))
    {
        if ((btnState & MK_MBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
            float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

            mCamera->Pitch(dy);
            mCamera->RotateY(dx);
            if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
            {
                mCameraFrustum0->Pitch(dy);
                mCameraFrustum1->Pitch(dy);
                mCameraFrustum2->Pitch(dy);
                mCameraFrustum3->Pitch(dy);
            }

        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }
}

void Engine::OnKeyboardInput(const GameTimer& gt)
{
    const float dt = gt.DeltaTime();

    float speed = dt * mCameraSpeed;

    if (GetAsyncKeyState('W') & 0x8000)
    {
        mCamera->Walk(10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            mCameraFrustum0->Walk(10.0f * speed);
            mCameraFrustum1->Walk(10.0f * speed);
            mCameraFrustum2->Walk(10.0f * speed);
            mCameraFrustum3->Walk(10.0f * speed);
        }
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        mCamera->Walk(-10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            mCameraFrustum0->Walk(-10.0f * speed);
            mCameraFrustum1->Walk(-10.0f * speed);
            mCameraFrustum2->Walk(-10.0f * speed);
            mCameraFrustum3->Walk(-10.0f * speed);
        }
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        mCamera->Strafe(-10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            mCameraFrustum0->Strafe(-10.0f * speed);
            mCameraFrustum1->Strafe(-10.0f * speed);
            mCameraFrustum2->Strafe(-10.0f * speed);
            mCameraFrustum3->Strafe(-10.0f * speed);
        }
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        mCamera->Strafe(10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            mCameraFrustum0->Strafe(10.0f * speed);
            mCameraFrustum1->Strafe(10.0f * speed);
            mCameraFrustum2->Strafe(10.0f * speed);
            mCameraFrustum3->Strafe(10.0f * speed);
        }
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
    {
        mCameraSpeed = 5.0f;
    }
    else
    {
        mCameraSpeed = 1.0f;
    }

    mCamera->UpdateViewMatrix();
    if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
    {
        mCameraFrustum0->UpdateViewMatrix();
        mCameraFrustum1->UpdateViewMatrix();
        mCameraFrustum2->UpdateViewMatrix();
        mCameraFrustum3->UpdateViewMatrix();
    }
}

void Engine::UpdateCamera(const GameTimer& gt)
{

}

void Engine::SetCamera2Scene3()
{
    mCamera2Scene3->LookAt(DirectX::XMFLOAT3(-20.0f, 20.0f, -20.0f), DirectX::XMFLOAT3(30.0f, 0.0f, 30.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCamera2Scene3->UpdateViewMatrix();
}

void Engine::SetCamera2Scene13()
{
    mCamera2Scene13->LookAt(DirectX::XMFLOAT3(40.0f, 40.0f, 40.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCamera2Scene13->UpdateViewMatrix();
}

void Engine::AnimateMaterials(const GameTimer& gt)
{
    auto animateMat = GeometryManager::mMaterials["metalAnimate"].get();

    float& tu = animateMat->MatTransform(3, 0);

    tu += 0.3f * gt.DeltaTime();

    if (tu >= 1.0f)
        tu -= 1.0f;

    animateMat->MatTransform(3, 0) = tu;

    // Material has changed, so need to update cbuffer.
    animateMat->NumFramesDirty = gNumFrameResources;
}

void Engine::UpdateInstanceData(const GameTimer& gt)
{
    DirectX::XMMATRIX view = mCamera->GetView();
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX invView = XMMatrixInverse(&det, view);

    auto currInstanceBuffer = CBManager::mCurrFrameResource->InstancingCB.get();
    {
        int visibleInstanceCount = 0;

        for (UINT i = 0; i < (UINT)instanceDataScene3.size(); ++i)
        {
            DirectX::XMMATRIX world = instanceDataScene3[i];
            DirectX::XMFLOAT4 Color = instancesData[i].Color;

            auto det2 = XMMatrixDeterminant(world);
            DirectX::XMMATRIX invWorld = DirectX::XMMatrixInverse(&det2, world);

            // View space to the object's local space.
            DirectX::XMMATRIX viewToLocal = DirectX::XMMatrixMultiply(invView, invWorld);

            // Transform the camera frustum from view space to the object's local space.
            DirectX::BoundingFrustum localSpaceFrustum;
            mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

            // Perform the box/frustum intersection test in local space.
            if (localSpaceFrustum.Contains(GeometryManager::mAllRitems[13]->Bounds) != DirectX::DISJOINT && isFrustumCullingScene3)
            {
                InstanceData data;
                DirectX::XMStoreFloat4x4(&data.WorldMatrix, XMMatrixTranspose(world));
                data.Color = Color;

                // Write the instance data to structured buffer for the visible objects.
                currInstanceBuffer->CopyData(visibleInstanceCount++, data);
            }
            else if (!isFrustumCullingScene3)
            {
                InstanceData data;
                DirectX::XMStoreFloat4x4(&data.WorldMatrix, XMMatrixTranspose(world));
                data.Color = Color;

                // Write the instance data to structured buffer for the visible objects.
                currInstanceBuffer->CopyData(visibleInstanceCount++, data);
            }
        }

        GeometryManager::mAllRitems[13]->InstanceCount = visibleInstanceCount;
    }
}

void Engine::UpdateInstanceDataScene13(const GameTimer& gt)
{
    DirectX::XMMATRIX view = mCamera->GetView();
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX invView = XMMatrixInverse(&det, view);

    auto currInstanceBuffer = CBManager::mCurrFrameResource->InstancingScene13CB.get();
    {
        int visibleInstanceCount = 0;

        DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&worldM);
        auto det2 = XMMatrixDeterminant(world);
        DirectX::XMMATRIX invWorld = DirectX::XMMatrixInverse(&det2, world);

        DirectX::XMMATRIX viewToLocal = DirectX::XMMatrixMultiply(invView, invWorld);

        DirectX::BoundingFrustum camFrustum;
        DirectX::BoundingFrustum::CreateFromMatrix(camFrustum, mCamera->GetProj());

        DirectX::BoundingFrustum localSpaceFrustum;
        camFrustum.Transform(localSpaceFrustum, viewToLocal);

        std::vector<GameObject*> visibleObjects;

        octreeScene13->OctreeCulling(octreeScene13->GetRoot(), localSpaceFrustum, visibleObjects);

        for (int i = 0; i < visibleObjects.size(); ++i)
        {
            InstanceDataGameObject data;
            DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
            worldMatrix = DirectX::XMLoadFloat4x4(&instancesDataOcTree[visibleObjects[i]->objectID].WorldMatrix);

            DirectX::XMStoreFloat4x4(&data.WorldMatrix, worldMatrix);
            data.Color = instancesDataOcTree[visibleObjects[i]->objectID].Color;
            currInstanceBuffer->CopyData(visibleInstanceCount++, data);
        }

        GeometryManager::mAllRitems[770]->InstanceCount = visibleInstanceCount;
    }
}

void Engine::UpdateLODScene3()
{
    float distance;
    DirectX::XMFLOAT3 cameraPosition = mCamera->GetPosition3f();
    DirectX::XMFLOAT3 objectPosition = DirectX::XMFLOAT3(0.0f, 0.0f, -10.0f);
    distance = sqrt((cameraPosition.x - objectPosition.x) * (cameraPosition.x - objectPosition.x)
        + (cameraPosition.y - objectPosition.y) * (cameraPosition.y - objectPosition.y)
        + (cameraPosition.z - objectPosition.z) * (cameraPosition.z - objectPosition.z));

    if (!isUsingManualLODScene3)
    {
        if (distance < 15.f)
        {
            LODScene3 = 3;
        }
        else if (distance < 35.f)
        {
            LODScene3 = 2;
        }
        else if (distance < 60.f)
        {
            LODScene3 = 1;
        }
        else
        {
            LODScene3 = 0;
        }
    }
    else LODScene3 = levelOfDetailsScene3;
}

void Engine::ChangeTileObjectTiles()
{
    if ((int)tilesCount != tilesCountInt)
    {
        tilesCount = tilesCountInt;
        for (auto& e : GeometryManager::mAllRitems)
        {
            if (e->Mat->Name == "tileCrate")
            {
                e->Mat->TilesCount = tilesCount;
                e->Mat->NumFramesDirty = gNumFrameResources;
            }
        }
    }
}

void Engine::LoadTextures()
{
    auto metalAnimateTex = std::make_unique<Texture>();
    metalAnimateTex->Name = "MetalTex";
    metalAnimateTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalTex.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateTex->Filename.c_str(),
        metalAnimateTex->Resource, metalAnimateTex->UploadHeap);

    GeometryManager::mTextures[metalAnimateTex->Name] = std::move(metalAnimateTex);

    auto metalAnimateNorm = std::make_unique<Texture>();
    metalAnimateNorm->Name = "MetalNorm";
    metalAnimateNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalNorm.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateNorm->Filename.c_str(),
        metalAnimateNorm->Resource, metalAnimateNorm->UploadHeap);

    GeometryManager::mTextures[metalAnimateNorm->Name] = std::move(metalAnimateNorm);

    auto metalAnimateDisplacement = std::make_unique<Texture>();
    metalAnimateDisplacement->Name = "MetalDisplacement";
    metalAnimateDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalDisplacement.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateDisplacement->Filename.c_str(),
        metalAnimateDisplacement->Resource, metalAnimateDisplacement->UploadHeap);

    GeometryManager::mTextures[metalAnimateDisplacement->Name] = std::move(metalAnimateDisplacement);

    auto metalRoughness = std::make_unique<Texture>();
    metalRoughness->Name = "MetalRoughness";
    metalRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalRoughness->Filename.c_str(),
        metalRoughness->Resource, metalRoughness->UploadHeap), true);

    GeometryManager::mTextures[metalRoughness->Name] = std::move(metalRoughness);

    auto metalAO = std::make_unique<Texture>();
    metalAO->Name = "MetalAO";
    metalAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAO->Filename.c_str(),
        metalAO->Resource, metalAO->UploadHeap), true);

    GeometryManager::mTextures[metalAO->Name] = std::move(metalAO);

    auto stoneTex = std::make_unique<Texture>();
    stoneTex->Name = "StoneTex";
    stoneTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneTex->Filename.c_str(),
        stoneTex->Resource, stoneTex->UploadHeap), false);
    GeometryManager::mTextures[stoneTex->Name] = std::move(stoneTex);

    auto stoneNorm = std::make_unique<Texture>();
    stoneNorm->Name = "StoneNorm";
    stoneNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneNorm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneNorm->Filename.c_str(),
        stoneNorm->Resource, stoneNorm->UploadHeap), false);
    GeometryManager::mTextures[stoneNorm->Name] = std::move(stoneNorm);

    auto stoneDisplacement = std::make_unique<Texture>();
    stoneDisplacement->Name = "StoneDisplacement";
    stoneDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneDisplacement.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneDisplacement->Filename.c_str(),
        stoneDisplacement->Resource, stoneDisplacement->UploadHeap), true);
    GeometryManager::mTextures[stoneDisplacement->Name] = std::move(stoneDisplacement);

    auto stoneRoughness = std::make_unique<Texture>();
    stoneRoughness->Name = "StoneRoughness";
    stoneRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneRoughness->Filename.c_str(),
        stoneRoughness->Resource, stoneRoughness->UploadHeap), true);
    GeometryManager::mTextures[stoneRoughness->Name] = std::move(stoneRoughness);

    auto stoneAO = std::make_unique<Texture>();
    stoneAO->Name = "StoneAO";
    stoneAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneAO->Filename.c_str(),
        stoneAO->Resource, stoneAO->UploadHeap), true);
    GeometryManager::mTextures[stoneAO->Name] = std::move(stoneAO);

    auto planetTex = std::make_unique<Texture>();
    planetTex->Name = "PlanetTex";
    planetTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PlanetTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), planetTex->Filename.c_str(),
        planetTex->Resource, planetTex->UploadHeap), true);
    GeometryManager::mTextures[planetTex->Name] = std::move(planetTex);

    auto particleTex = std::make_unique<Texture>();
    particleTex->Name = "ParticleTex";
    particleTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\ParticleTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particleTex->Filename.c_str(),
        particleTex->Resource, particleTex->UploadHeap), true);
    GeometryManager::mTextures[particleTex->Name] = std::move(particleTex);

    auto particle2Tex = std::make_unique<Texture>();
    particle2Tex->Name = "Particle2Tex";
    particle2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Particle2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particle2Tex->Filename.c_str(),
        particle2Tex->Resource, particle2Tex->UploadHeap), true);
    GeometryManager::mTextures[particle2Tex->Name] = std::move(particle2Tex);

    auto shadow1Tex = std::make_unique<Texture>();
    shadow1Tex->Name = "Shadow1Tex";
    shadow1Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Shadow1Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow1Tex->Filename.c_str(),
        shadow1Tex->Resource, shadow1Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow1Tex->Name] = std::move(shadow1Tex);

    auto shadow2Tex = std::make_unique<Texture>();
    shadow2Tex->Name = "Shadow2Tex";
    shadow2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow2Tex->Filename.c_str(),
        shadow2Tex->Resource, shadow2Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow2Tex->Name] = std::move(shadow2Tex);

    auto shadow3Tex = std::make_unique<Texture>();
    shadow3Tex->Name = "Shadow3Tex";
    shadow3Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow3Tex->Filename.c_str(),
        shadow3Tex->Resource, shadow3Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow3Tex->Name] = std::move(shadow3Tex);

    auto bonfireTex = std::make_unique<Texture>();
    bonfireTex->Name = "BonfireTex";
    bonfireTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\BonfireTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), bonfireTex->Filename.c_str(),
        bonfireTex->Resource, bonfireTex->UploadHeap), true);
    GeometryManager::mTextures[bonfireTex->Name] = std::move(bonfireTex);

    auto particlesSmokeTex = std::make_unique<Texture>();
    particlesSmokeTex->Name = "ParticleSmokeTex";
    particlesSmokeTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\ParticleSmokeTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particlesSmokeTex->Filename.c_str(),
        particlesSmokeTex->Resource, particlesSmokeTex->UploadHeap), true);
    GeometryManager::mTextures[particlesSmokeTex->Name] = std::move(particlesSmokeTex);

    auto grassTex = std::make_unique<Texture>();
    grassTex->Name = "GrassTex";
    grassTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\GrassTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), grassTex->Filename.c_str(),
        grassTex->Resource, grassTex->UploadHeap), true);
    GeometryManager::mTextures[grassTex->Name] = std::move(grassTex);

    auto skyboxTex = std::make_unique<Texture>();
    skyboxTex->Name = "SkyboxTex";
    skyboxTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\SkyboxTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), skyboxTex->Filename.c_str(),
        skyboxTex->Resource, skyboxTex->UploadHeap), true);
    GeometryManager::mTextures[skyboxTex->Name] = std::move(skyboxTex);

    auto PBR1Tex = std::make_unique<Texture>();
    PBR1Tex->Name = "PBR1Tex";
    PBR1Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Tex->Filename.c_str(),
        PBR1Tex->Resource, PBR1Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR1Tex->Name] = std::move(PBR1Tex);

    auto PBR1Norm = std::make_unique<Texture>();
    PBR1Norm->Name = "PBR1Norm";
    PBR1Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Norm->Filename.c_str(),
        PBR1Norm->Resource, PBR1Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR1Norm->Name] = std::move(PBR1Norm);

    auto PBR1AO = std::make_unique<Texture>();
    PBR1AO->Name = "PBR1AO";
    PBR1AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1AO->Filename.c_str(),
        PBR1AO->Resource, PBR1AO->UploadHeap), true);
    GeometryManager::mTextures[PBR1AO->Name] = std::move(PBR1AO);

    auto PBR1Metallic = std::make_unique<Texture>();
    PBR1Metallic->Name = "PBR1Metallic";
    PBR1Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Metallic->Filename.c_str(),
        PBR1Metallic->Resource, PBR1Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR1Metallic->Name] = std::move(PBR1Metallic);

    auto PBR1Roughness = std::make_unique<Texture>();
    PBR1Roughness->Name = "PBR1Roughness";
    PBR1Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Roughness->Filename.c_str(),
        PBR1Roughness->Resource, PBR1Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR1Roughness->Name] = std::move(PBR1Roughness);

    auto PBR2Tex = std::make_unique<Texture>();
    PBR2Tex->Name = "PBR2Tex";
    PBR2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Tex->Filename.c_str(),
        PBR2Tex->Resource, PBR2Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR2Tex->Name] = std::move(PBR2Tex);

    auto PBR2Norm = std::make_unique<Texture>();
    PBR2Norm->Name = "PBR2Norm";
    PBR2Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Norm->Filename.c_str(),
        PBR2Norm->Resource, PBR2Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR2Norm->Name] = std::move(PBR2Norm);

    auto PBR2AO = std::make_unique<Texture>();
    PBR2AO->Name = "PBR2AO";
    PBR2AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2AO->Filename.c_str(),
        PBR2AO->Resource, PBR2AO->UploadHeap), true);
    GeometryManager::mTextures[PBR2AO->Name] = std::move(PBR2AO);

    auto PBR2Metallic = std::make_unique<Texture>();
    PBR2Metallic->Name = "PBR2Metallic";
    PBR2Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Metallic->Filename.c_str(),
        PBR2Metallic->Resource, PBR2Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR2Metallic->Name] = std::move(PBR2Metallic);

    auto PBR2Roughness = std::make_unique<Texture>();
    PBR2Roughness->Name = "PBR2Roughness";
    PBR2Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Roughness->Filename.c_str(),
        PBR2Roughness->Resource, PBR2Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR2Roughness->Name] = std::move(PBR2Roughness);

    auto PBR3Tex = std::make_unique<Texture>();
    PBR3Tex->Name = "PBR3Tex";
    PBR3Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Tex->Filename.c_str(),
        PBR3Tex->Resource, PBR3Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR3Tex->Name] = std::move(PBR3Tex);

    auto PBR3Norm = std::make_unique<Texture>();
    PBR3Norm->Name = "PBR3Norm";
    PBR3Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Norm->Filename.c_str(),
        PBR3Norm->Resource, PBR3Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR3Norm->Name] = std::move(PBR3Norm);

    auto PBR3AO = std::make_unique<Texture>();
    PBR3AO->Name = "PBR3AO";
    PBR3AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3AO->Filename.c_str(),
        PBR3AO->Resource, PBR3AO->UploadHeap), true);
    GeometryManager::mTextures[PBR3AO->Name] = std::move(PBR3AO);

    auto PBR3Metallic = std::make_unique<Texture>();
    PBR3Metallic->Name = "PBR3Metallic";
    PBR3Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Metallic->Filename.c_str(),
        PBR3Metallic->Resource, PBR3Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR3Metallic->Name] = std::move(PBR3Metallic);

    auto PBR3Roughness = std::make_unique<Texture>();
    PBR3Roughness->Name = "PBR3Roughness";
    PBR3Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Roughness->Filename.c_str(),
        PBR3Roughness->Resource, PBR3Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR3Roughness->Name] = std::move(PBR3Roughness);

    auto PBR4Tex = std::make_unique<Texture>();
    PBR4Tex->Name = "PBR4Tex";
    PBR4Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Tex->Filename.c_str(),
        PBR4Tex->Resource, PBR4Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR4Tex->Name] = std::move(PBR4Tex);

    auto PBR4Norm = std::make_unique<Texture>();
    PBR4Norm->Name = "PBR4Norm";
    PBR4Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Norm->Filename.c_str(),
        PBR4Norm->Resource, PBR4Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR4Norm->Name] = std::move(PBR4Norm);

    auto PBR4AO = std::make_unique<Texture>();
    PBR4AO->Name = "PBR4AO";
    PBR4AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4AO->Filename.c_str(),
        PBR4AO->Resource, PBR4AO->UploadHeap), true);
    GeometryManager::mTextures[PBR4AO->Name] = std::move(PBR4AO);

    auto PBR4Metallic = std::make_unique<Texture>();
    PBR4Metallic->Name = "PBR4Metallic";
    PBR4Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Metallic->Filename.c_str(),
        PBR4Metallic->Resource, PBR4Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR4Metallic->Name] = std::move(PBR4Metallic);

    auto PBR4Roughness = std::make_unique<Texture>();
    PBR4Roughness->Name = "PBR4Roughness";
    PBR4Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Roughness->Filename.c_str(),
        PBR4Roughness->Resource, PBR4Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR4Roughness->Name] = std::move(PBR4Roughness);

    auto PBR5Tex = std::make_unique<Texture>();
    PBR5Tex->Name = "PBR5Tex";
    PBR5Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Tex->Filename.c_str(),
        PBR5Tex->Resource, PBR5Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR5Tex->Name] = std::move(PBR5Tex);

    auto PBR5Norm = std::make_unique<Texture>();
    PBR5Norm->Name = "PBR5Norm";
    PBR5Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Norm->Filename.c_str(),
        PBR5Norm->Resource, PBR5Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR5Norm->Name] = std::move(PBR5Norm);

    auto PBR5AO = std::make_unique<Texture>();
    PBR5AO->Name = "PBR5AO";
    PBR5AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5AO->Filename.c_str(),
        PBR5AO->Resource, PBR5AO->UploadHeap), true);
    GeometryManager::mTextures[PBR5AO->Name] = std::move(PBR5AO);

    auto PBR5Metallic = std::make_unique<Texture>();
    PBR5Metallic->Name = "PBR5Metallic";
    PBR5Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Metallic->Filename.c_str(),
        PBR5Metallic->Resource, PBR5Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR5Metallic->Name] = std::move(PBR5Metallic);

    auto PBR5Roughness = std::make_unique<Texture>();
    PBR5Roughness->Name = "PBR5Roughness";
    PBR5Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Roughness->Filename.c_str(),
        PBR5Roughness->Resource, PBR5Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR5Roughness->Name] = std::move(PBR5Roughness);

    auto IrradianceMap = std::make_unique<Texture>();
    IrradianceMap->Name = "IrradianceMap";
    IrradianceMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\IrradianceMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), IrradianceMap->Filename.c_str(),
        IrradianceMap->Resource, IrradianceMap->UploadHeap), true);
    GeometryManager::mTextures[IrradianceMap->Name] = std::move(IrradianceMap);

    auto PreFilteredEnvMap = std::make_unique<Texture>();
    PreFilteredEnvMap->Name = "PreFilteredEnvMap";
    PreFilteredEnvMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PreFilteredEnvMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PreFilteredEnvMap->Filename.c_str(),
        PreFilteredEnvMap->Resource, PreFilteredEnvMap->UploadHeap), true);
    GeometryManager::mTextures[PreFilteredEnvMap->Name] = std::move(PreFilteredEnvMap);

    auto IntegrationMap = std::make_unique<Texture>();
    IntegrationMap->Name = "IntegrationMap";
    IntegrationMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\IntegrationMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), IntegrationMap->Filename.c_str(),
        IntegrationMap->Resource, IntegrationMap->UploadHeap), true);
    GeometryManager::mTextures[IntegrationMap->Name] = std::move(IntegrationMap);

    auto TerrainTex = std::make_unique<Texture>();
    TerrainTex->Name = "TerrainTex";
    TerrainTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\TerrainTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), TerrainTex->Filename.c_str(),
        TerrainTex->Resource, TerrainTex->UploadHeap), true);
    GeometryManager::mTextures[TerrainTex->Name] = std::move(TerrainTex);

    auto TerrainHeight = std::make_unique<Texture>();
    TerrainHeight->Name = "TerrainHeight";
    TerrainHeight->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\TerrainHeight.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), TerrainHeight->Filename.c_str(),
        TerrainHeight->Resource, TerrainHeight->UploadHeap), true);
    GeometryManager::mTextures[TerrainHeight->Name] = std::move(TerrainHeight);

    auto RainTex = std::make_unique<Texture>();
    RainTex->Name = "RainTex";
    RainTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\RainTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), RainTex->Filename.c_str(),
        RainTex->Resource, RainTex->UploadHeap), true);
    GeometryManager::mTextures[RainTex->Name] = std::move(RainTex);

    auto RainCircleTex = std::make_unique<Texture>();
    RainCircleTex->Name = "RainCircleTex";
    RainCircleTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\RainCircleTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), RainCircleTex->Filename.c_str(),
        RainCircleTex->Resource, RainCircleTex->UploadHeap), true);
    GeometryManager::mTextures[RainCircleTex->Name] = std::move(RainCircleTex);

    auto Terrain2Tex = std::make_unique<Texture>();
    Terrain2Tex->Name = "Terrain2Tex";
    Terrain2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Tex->Filename.c_str(),
        Terrain2Tex->Resource, Terrain2Tex->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Tex->Name] = std::move(Terrain2Tex);

    auto Terrain2Norm = std::make_unique<Texture>();
    Terrain2Norm->Name = "Terrain2Norm";
    Terrain2Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Normal.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Norm->Filename.c_str(),
        Terrain2Norm->Resource, Terrain2Norm->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Norm->Name] = std::move(Terrain2Norm);

    auto Terrain2Height = std::make_unique<Texture>();
    Terrain2Height->Name = "Terrain2Height";
    Terrain2Height->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Height.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Height->Filename.c_str(),
        Terrain2Height->Resource, Terrain2Height->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Height->Name] = std::move(Terrain2Height);

    auto HamburgerTex = std::make_unique<Texture>();
    HamburgerTex->Name = "HamburgerTex";
    HamburgerTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerTex->Filename.c_str(),
        HamburgerTex->Resource, HamburgerTex->UploadHeap), true);
    GeometryManager::mTextures[HamburgerTex->Name] = std::move(HamburgerTex);

    auto HamburgerNorm = std::make_unique<Texture>();
    HamburgerNorm->Name = "HamburgerNorm";
    HamburgerNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerNormal.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerNorm->Filename.c_str(),
        HamburgerNorm->Resource, HamburgerNorm->UploadHeap), true);
    GeometryManager::mTextures[HamburgerNorm->Name] = std::move(HamburgerNorm);

    auto HamburgerAO = std::make_unique<Texture>();
    HamburgerAO->Name = "HamburgerAO";
    HamburgerAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerAO->Filename.c_str(),
        HamburgerAO->Resource, HamburgerAO->UploadHeap), true);
    GeometryManager::mTextures[HamburgerAO->Name] = std::move(HamburgerAO);

    auto HamburgerMetallic = std::make_unique<Texture>();
    HamburgerMetallic->Name = "HamburgerMetallic";
    HamburgerMetallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerMetallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerMetallic->Filename.c_str(),
        HamburgerMetallic->Resource, HamburgerMetallic->UploadHeap), true);
    GeometryManager::mTextures[HamburgerMetallic->Name] = std::move(HamburgerMetallic);

    auto HamburgerRoughness = std::make_unique<Texture>();
    HamburgerRoughness->Name = "HamburgerRoughness";
    HamburgerRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerRoughness->Filename.c_str(),
        HamburgerRoughness->Resource, HamburgerRoughness->UploadHeap), true);
    GeometryManager::mTextures[HamburgerRoughness->Name] = std::move(HamburgerRoughness);
}

void Engine::BuildShadowMaps()
{
    mShadowMap256 = std::make_unique<ShadowMap>(md3dDevice.Get(), 256, 256);
    mShadowMap512 = std::make_unique<ShadowMap>(md3dDevice.Get(), 512, 512);
    mShadowMap1024 = std::make_unique<ShadowMap>(md3dDevice.Get(), 1024, 1024);
    mShadowMap2048 = std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048);

    mShadowMapScene6 = std::make_unique<ShadowMap>(md3dDevice.Get(), 2048, 2048);
}

void Engine::BuildDescriptorHeaps()
{
    //
    // Create the SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 75;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

    mSrvHeapAllocator.Create(md3dDevice.Get(), mSrvHeap.Get());

    //
    // Create particles SRV/UAV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvParticlesHeapDesc = {};
    srvParticlesHeapDesc.NumDescriptors = 40;
    srvParticlesHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvParticlesHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvParticlesHeapDesc, IID_PPV_ARGS(&mParticlesSrvUavHeap)));

    mSrvHeapAllocator2.Create(md3dDevice.Get(), mParticlesSrvUavHeap.Get());

    //
    // Create sponza SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvSponzaHeapDesc = {};
    srvSponzaHeapDesc.NumDescriptors = 35;
    srvSponzaHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvSponzaHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvSponzaHeapDesc, IID_PPV_ARGS(&mSponzaSrvHeap)));

    mSrvHeapAllocatorSponza.Create(md3dDevice.Get(), mSponzaSrvHeap.Get());

    BuildShadowMapsDescriptors();
}

void Engine::BuildShadowMapsDescriptors()
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
    md3dDevice->CreateShaderResourceView(ShadowTex1.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = ShadowTex2->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(ShadowTex2.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = ShadowTex3->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(ShadowTex3.Get(), &srvDesc1, hDescriptor);
}

void Engine::UploadTextures()
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
    md3dDevice->CreateShaderResourceView(MetalTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalNorm->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(MetalNorm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalDisplacement->GetDesc().Format;
    srvDesc1.Texture2D.MipLevels = MetalDisplacement->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(MetalDisplacement.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalRoughness->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(MetalRoughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc1.Format = MetalAO->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(MetalAO.Get(), &srvDesc1, hDescriptor);

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
    md3dDevice->CreateShaderResourceView(StoneTex.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneNorm->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(StoneNorm.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneDisplacement->GetDesc().Format;
    srvDesc3.Texture2D.MipLevels = StoneDisplacement->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(StoneDisplacement.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneRoughness->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(StoneRoughness.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDesc3.Format = StoneAO->GetDesc().Format;
    md3dDevice->CreateShaderResourceView(StoneAO.Get(), &srvDesc3, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

void Engine::UploadTextures2()
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
    md3dDevice->CreateShaderResourceView(PlanetTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto SkyboxTex = GeometryManager::mTextures["SkyboxTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = SkyboxTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = SkyboxTex->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(SkyboxTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(5, mCbvSrvDescriptorSize);

    auto ParticleTex = GeometryManager::mTextures["ParticleTex"]->Resource;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptorParticles(mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptorParticles.Offset(21, mCbvSrvDescriptorSize);

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(ParticleTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto Particle2Tex = GeometryManager::mTextures["Particle2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(Particle2Tex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto ParticleSmokeTex = GeometryManager::mTextures["ParticleSmokeTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleSmokeTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(ParticleSmokeTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto BonfireTex = GeometryManager::mTextures["BonfireTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = BonfireTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(BonfireTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto GrassTex = GeometryManager::mTextures["GrassTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = GrassTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(GrassTex.Get(), &srvDesc1, hDescriptorParticles);

    auto PBR1Tex = GeometryManager::mTextures["PBR1Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR1Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Norm = GeometryManager::mTextures["PBR1Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR1Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Metallic = GeometryManager::mTextures["PBR1Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR1Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1Roughness = GeometryManager::mTextures["PBR1Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR1Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR1AO = GeometryManager::mTextures["PBR1AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR1AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR1AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Tex = GeometryManager::mTextures["PBR2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR2Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Norm = GeometryManager::mTextures["PBR2Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR2Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Metallic = GeometryManager::mTextures["PBR2Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR2Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2Roughness = GeometryManager::mTextures["PBR2Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR2Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR2AO = GeometryManager::mTextures["PBR2AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR2AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR2AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Tex = GeometryManager::mTextures["PBR3Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR3Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Norm = GeometryManager::mTextures["PBR3Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR3Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Metallic = GeometryManager::mTextures["PBR3Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR3Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3Roughness = GeometryManager::mTextures["PBR3Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR3Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR3AO = GeometryManager::mTextures["PBR3AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR3AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR3AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Tex = GeometryManager::mTextures["PBR4Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR4Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Norm = GeometryManager::mTextures["PBR4Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR4Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Metallic = GeometryManager::mTextures["PBR4Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR4Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4Roughness = GeometryManager::mTextures["PBR4Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR4Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR4AO = GeometryManager::mTextures["PBR4AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR4AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR4AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Tex = GeometryManager::mTextures["PBR5Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR5Tex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Norm = GeometryManager::mTextures["PBR5Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR5Norm.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Metallic = GeometryManager::mTextures["PBR5Metallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Metallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR5Metallic.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5Roughness = GeometryManager::mTextures["PBR5Roughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5Roughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR5Roughness.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PBR5AO = GeometryManager::mTextures["PBR5AO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PBR5AO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PBR5AO.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto IrradianceMap = GeometryManager::mTextures["IrradianceMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = IrradianceMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(IrradianceMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto PreFilteredEnvMap = GeometryManager::mTextures["PreFilteredEnvMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PreFilteredEnvMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PreFilteredEnvMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto IntegrationMap = GeometryManager::mTextures["IntegrationMap"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = IntegrationMap->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(IntegrationMap.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor2(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor2.Offset(59, mCbvSrvDescriptorSize);

    auto Terrain2Tex = GeometryManager::mTextures["Terrain2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Tex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(Terrain2Tex.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto Terrain2Norm = GeometryManager::mTextures["Terrain2Norm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Norm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(Terrain2Norm.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto Terrain2Height = GeometryManager::mTextures["Terrain2Height"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = Terrain2Height->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(Terrain2Height.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);
}

void Engine::UploadTextures3()
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
    md3dDevice->CreateShaderResourceView(TerrainTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto TerrainHeight = GeometryManager::mTextures["TerrainHeight"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = TerrainHeight->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = TerrainHeight->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(TerrainHeight.Get(), &srvDesc1, hDescriptor);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptorParticles(mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptorParticles.Offset(27, mCbvSrvDescriptorSize);

    auto RainTex = GeometryManager::mTextures["RainTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = RainTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = RainTex->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(RainTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto RainCircleTex = GeometryManager::mTextures["RainCircleTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = RainCircleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = RainCircleTex->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(RainCircleTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor2(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor2.Offset(62, mCbvSrvDescriptorSize);

    auto HamburgerTex = GeometryManager::mTextures["HamburgerTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerTex->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(HamburgerTex.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerNorm = GeometryManager::mTextures["HamburgerNorm"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerNorm->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerNorm->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(HamburgerNorm.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerMetallic = GeometryManager::mTextures["HamburgerMetallic"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerMetallic->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerMetallic->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(HamburgerMetallic.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerRoughness = GeometryManager::mTextures["HamburgerRoughness"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerRoughness->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerRoughness->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(HamburgerRoughness.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);

    auto HamburgerAO = GeometryManager::mTextures["HamburgerAO"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = HamburgerAO->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = HamburgerAO->GetDesc().MipLevels;
    md3dDevice->CreateShaderResourceView(HamburgerAO.Get(), &srvDesc1, hDescriptor2);

    hDescriptor2.Offset(1, mCbvSrvDescriptorSize);
}


void Engine::BuildRootSignature()
{
    RootSignatureManager::BuildRootSignature(md3dDevice, mRootSignatures);
}


void Engine::BuildShadersAndInputLayout()
{
    InputLayoutShaderManager::BuildShadersAndInputLayout(mInputLayouts, mShaders);
}

void Engine::BuildSkyboxGeometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData skysphere = geoGen.CreateSphere(0.5f, 20, 20, 0.0f, 0.0f, 0.0f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry skyboxSubmesh;
    skyboxSubmesh.IndexCount = (UINT)skysphere.Indices32.size();
    skyboxSubmesh.StartIndexLocation = totalIndexCount;
    skyboxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += skyboxSubmesh.IndexCount;
    totalVertexCount += skysphere.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < skysphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = skysphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = skysphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = skysphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = skysphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += skysphere.Vertices.size();

    indices.insert(indices.end(), std::begin(skysphere.GetIndices16()), std::end(skysphere.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "skyboxGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["skybox"] = skyboxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box2 = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box3 = geoGen.CreateBoxTiling(5.f, 5.f, 5.f, 5, 1);
    GeometryGenerator::MeshData box4 = geoGen.CreateBoxTiling(1.f, 1.f, 1.f, 1, 1);
    GeometryGenerator::MeshData boxFPS = geoGen.CreateBoxTiling(2.f, 2.f, 2.f, 5, 1);
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.0f, 7, 7, 0.f, 0.f, 0.f);
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(200.0f, 200.0f, 100, 100);
    GeometryGenerator::MeshData cone = geoGen.CreateCone(1.0f, 0.2f, 20);
    GeometryGenerator::MeshData skyBox = geoGen.CreateBackSphere(1.f, 50.f, 50.f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry boxSubmesh;
    boxSubmesh.IndexCount = (UINT)box.Indices32.size();
    boxSubmesh.StartIndexLocation = totalIndexCount;
    boxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh.IndexCount;
    totalVertexCount += box.Vertices.size();

    SubmeshGeometry boxSubmesh2;
    boxSubmesh2.IndexCount = (UINT)box2.Indices32.size();
    boxSubmesh2.StartIndexLocation = totalIndexCount;
    boxSubmesh2.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh2.IndexCount;
    totalVertexCount += box2.Vertices.size();

    SubmeshGeometry boxSubmesh3;
    boxSubmesh3.IndexCount = (UINT)box3.Indices32.size();
    boxSubmesh3.StartIndexLocation = totalIndexCount;
    boxSubmesh3.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh3.IndexCount;
    totalVertexCount += box3.Vertices.size();

    SubmeshGeometry boxSubmesh4;
    boxSubmesh4.IndexCount = (UINT)box4.Indices32.size();
    boxSubmesh4.StartIndexLocation = totalIndexCount;
    boxSubmesh4.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh4.IndexCount;
    totalVertexCount += box4.Vertices.size();

    SubmeshGeometry boxSubmeshFPS;
    boxSubmeshFPS.IndexCount = (UINT)boxFPS.Indices32.size();
    boxSubmeshFPS.StartIndexLocation = totalIndexCount;
    boxSubmeshFPS.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmeshFPS.IndexCount;
    totalVertexCount += boxFPS.Vertices.size();

    SubmeshGeometry pointLight1Submesh;
    pointLight1Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight1Submesh.StartIndexLocation = totalIndexCount;
    pointLight1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight1Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight2Submesh;
    pointLight2Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight2Submesh.StartIndexLocation = totalIndexCount;
    pointLight2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight2Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight3Submesh;
    pointLight3Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight3Submesh.StartIndexLocation = totalIndexCount;
    pointLight3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight3Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry gridSubmesh;
    gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
    gridSubmesh.StartIndexLocation = totalIndexCount;
    gridSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += gridSubmesh.IndexCount;
    totalVertexCount += grid.Vertices.size();

    SubmeshGeometry coneSubmesh;
    coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
    coneSubmesh.StartIndexLocation = totalIndexCount;
    coneSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += coneSubmesh.IndexCount;
    totalVertexCount += cone.Vertices.size();

    SubmeshGeometry skyboxSubmesh;
    skyboxSubmesh.IndexCount = (UINT)skyBox.Indices32.size();
    skyboxSubmesh.StartIndexLocation = totalIndexCount;
    skyboxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += skyboxSubmesh.IndexCount;
    totalVertexCount += skyBox.Vertices.size();

    std::vector<VertexLightStage> verticesLightStage = 
    {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }
    };

    const UINT vbByteSizeLight = (UINT)verticesLightStage.size() * sizeof(VertexLightStage);

    auto screenQuad = std::make_unique<MeshGeometry>();
    screenQuad->Name = "screenQuad";

    ThrowIfFailed(D3DCreateBlob(vbByteSizeLight, &screenQuad->VertexBufferCPU));
    CopyMemory(screenQuad->VertexBufferCPU->GetBufferPointer(), verticesLightStage.data(), vbByteSizeLight);

    screenQuad->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), verticesLightStage.data(), vbByteSizeLight, screenQuad->VertexBufferUploader);

    screenQuad->VertexByteStride = sizeof(VertexLightStage);
    screenQuad->VertexBufferByteSize = vbByteSizeLight;

    GeometryManager::mGeometries[screenQuad->Name] = std::move(screenQuad);

    std::vector<Vertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;
    // Full screen quad

    for (size_t i = 0; i < box.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box.Vertices[i].TangentU;
    }
    totalVertexCount2 += box.Vertices.size();
    for (size_t i = 0; i < box2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box2.Vertices[i].TangentU;
    }
    totalVertexCount2 += box2.Vertices.size();
    for (size_t i = 0; i < box3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box3.Vertices[i].TangentU;
    }
    totalVertexCount2 += box3.Vertices.size();
    for (size_t i = 0; i < box4.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box4.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box4.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box4.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box4.Vertices[i].TangentU;
    }
    totalVertexCount2 += box4.Vertices.size();
    for (size_t i = 0; i < boxFPS.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxFPS.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxFPS.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxFPS.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxFPS.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxFPS.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = grid.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = grid.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = grid.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = grid.Vertices[i].TangentU;
    }
    totalVertexCount2 += grid.Vertices.size();
    for (size_t i = 0; i < cone.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = cone.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = cone.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = cone.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = cone.Vertices[i].TangentU;
    }
    totalVertexCount2 += cone.Vertices.size();
    for (size_t i = 0; i < skyBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = skyBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = skyBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = skyBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = skyBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += skyBox.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
    indices.insert(indices.end(), std::begin(box2.GetIndices16()), std::end(box2.GetIndices16()));
    indices.insert(indices.end(), std::begin(box3.GetIndices16()), std::end(box3.GetIndices16()));
    indices.insert(indices.end(), std::begin(box4.GetIndices16()), std::end(box4.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxFPS.GetIndices16()), std::end(boxFPS.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
    indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));
    indices.insert(indices.end(), std::begin(skyBox.GetIndices16()), std::end(skyBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "boxGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["box"] = boxSubmesh;
    geo->DrawArgs["box2"] = boxSubmesh2;
    geo->DrawArgs["box3"] = boxSubmesh3;
    geo->DrawArgs["box4"] = boxSubmesh4;
    geo->DrawArgs["boxFPS"] = boxSubmeshFPS;
    geo->DrawArgs["pointLight1"] = pointLight1Submesh;
    geo->DrawArgs["pointLight2"] = pointLight2Submesh;
    geo->DrawArgs["pointLight3"] = pointLight3Submesh;
    geo->DrawArgs["grid"] = gridSubmesh;
    geo->DrawArgs["cone"] = coneSubmesh;
    geo->DrawArgs["skybox"] = skyboxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene3Geometry()
{
    GeometryGenerator geoGen;
   
    GeometryGenerator::MeshData instancingBox = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry boxSubmesh;
    boxSubmesh.IndexCount = (UINT)instancingBox.Indices32.size();
    boxSubmesh.StartIndexLocation = totalIndexCount;
    boxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh.IndexCount;
    totalVertexCount += instancingBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < instancingBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = instancingBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = instancingBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = instancingBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = instancingBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += instancingBox.Vertices.size();

    DirectX::BoundingBox bounds;
    bounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    bounds.Extents = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

    indices.insert(indices.end(), std::begin(instancingBox.GetIndices16()), std::end(instancingBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene3Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;
    boxSubmesh.Bounds = bounds;

    geo->DrawArgs["boxInstancing"] = boxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);

    instanceDataScene3 = std::vector<DirectX::XMMATRIX>(729);
    for (int i = 0; i < 729; ++i)
    {
        int x = i % 27;
        int y = i / 27;
        int index = 0;
        if (y >= x)
            index = y * y + x;
        else index = (x + 1) * (x + 1) - y - 1;

        instanceDataScene3[index] = DirectX::XMMatrixTranslation((float)x * 2, 0, (float)y * 2);
    }
}

void Engine::BuildScene3LODGeometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereLOD3 = geoGen.CreateSphere(3.0f, 20, 20, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD2 = geoGen.CreateSphere(3.0f, 12, 12, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD1 = geoGen.CreateSphere(3.0f, 8, 8, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD0 = geoGen.CreateSphere(3.0f, 5, 5, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereLOD3Submesh;
    sphereLOD3Submesh.IndexCount = (UINT)sphereLOD3.Indices32.size();
    sphereLOD3Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD3Submesh.IndexCount;
    totalVertexCount += sphereLOD3.Vertices.size();

    SubmeshGeometry sphereLOD2Submesh;
    sphereLOD2Submesh.IndexCount = (UINT)sphereLOD2.Indices32.size();
    sphereLOD2Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD2Submesh.IndexCount;
    totalVertexCount += sphereLOD2.Vertices.size();

    SubmeshGeometry sphereLOD1Submesh;
    sphereLOD1Submesh.IndexCount = (UINT)sphereLOD1.Indices32.size();
    sphereLOD1Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD1Submesh.IndexCount;
    totalVertexCount += sphereLOD1.Vertices.size();

    SubmeshGeometry sphereLOD0Submesh;
    sphereLOD0Submesh.IndexCount = (UINT)sphereLOD0.Indices32.size();
    sphereLOD0Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD0Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD0Submesh.IndexCount;
    totalVertexCount += sphereLOD0.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereLOD3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD3.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD3.Vertices.size();
    for (size_t i = 0; i < sphereLOD2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD2.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD2.Vertices.size();
    for (size_t i = 0; i < sphereLOD1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD1.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD1.Vertices.size();
    for (size_t i = 0; i < sphereLOD0.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD0.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD0.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD0.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD0.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD0.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereLOD3.GetIndices16()), std::end(sphereLOD3.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD2.GetIndices16()), std::end(sphereLOD2.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD1.GetIndices16()), std::end(sphereLOD1.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD0.GetIndices16()), std::end(sphereLOD0.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene3LODGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphereLOD3"] = sphereLOD3Submesh;
    geo->DrawArgs["sphereLOD2"] = sphereLOD2Submesh;
    geo->DrawArgs["sphereLOD1"] = sphereLOD1Submesh;
    geo->DrawArgs["sphereLOD0"] = sphereLOD0Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene4Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData animateBox = geoGen.CreateBox(2.0f, 2.0f, 2.0f, 0);
    GeometryGenerator::MeshData tilingBox = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 0, 1);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry animateBoxSubmesh;
    animateBoxSubmesh.IndexCount = (UINT)animateBox.Indices32.size();
    animateBoxSubmesh.StartIndexLocation = totalIndexCount;
    animateBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += animateBoxSubmesh.IndexCount;
    totalVertexCount += animateBox.Vertices.size();

    SubmeshGeometry tilingBoxSubmesh;
    tilingBoxSubmesh.IndexCount = (UINT)tilingBox.Indices32.size();
    tilingBoxSubmesh.StartIndexLocation = totalIndexCount;
    tilingBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += tilingBoxSubmesh.IndexCount;
    totalVertexCount += tilingBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < animateBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = animateBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = animateBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = animateBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = animateBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += animateBox.Vertices.size();
    for (size_t i = 0; i < tilingBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = tilingBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = tilingBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = tilingBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = tilingBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += tilingBox.Vertices.size();

    indices.insert(indices.end(), std::begin(animateBox.GetIndices16()), std::end(animateBox.GetIndices16()));
    indices.insert(indices.end(), std::begin(tilingBox.GetIndices16()), std::end(tilingBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene4Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["animateBox"] = animateBoxSubmesh;
    geo->DrawArgs["tilingBox"] = tilingBoxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene5Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData tessellationBox = geoGen.CreateBox(5.0f, 5.0f, 5.0f, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry tessellationBoxSubmesh;
    tessellationBoxSubmesh.IndexCount = (UINT)tessellationBox.Indices32.size();
    tessellationBoxSubmesh.StartIndexLocation = totalIndexCount;
    tessellationBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += tessellationBoxSubmesh.IndexCount;
    totalVertexCount += tessellationBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < tessellationBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = tessellationBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = tessellationBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = tessellationBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = tessellationBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += tessellationBox.Vertices.size();

    indices.insert(indices.end(), std::begin(tessellationBox.GetIndices16()), std::end(tessellationBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene5Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["tessellationBox"] = tessellationBoxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene6Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData grassPlatform = geoGen.CreateBoxTiling(100.0f, 0.1f, 100.0f, 0, 10);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry grassPlatformSubmesh;
    grassPlatformSubmesh.IndexCount = (UINT)grassPlatform.Indices32.size();
    grassPlatformSubmesh.StartIndexLocation = totalIndexCount;
    grassPlatformSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += grassPlatformSubmesh.IndexCount;
    totalVertexCount += grassPlatform.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < grassPlatform.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = grassPlatform.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = grassPlatform.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = grassPlatform.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = grassPlatform.Vertices[i].TangentU;
    }
    totalVertexCount2 += grassPlatform.Vertices.size();

    indices.insert(indices.end(), std::begin(grassPlatform.GetIndices16()), std::end(grassPlatform.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene6GrassPlatform";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["GrassPlatform"] = grassPlatformSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene7Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData boxMesh1 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);
    GeometryGenerator::MeshData boxMesh2 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);
    GeometryGenerator::MeshData boxMesh3 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry box1Submesh;
    box1Submesh.IndexCount = (UINT)boxMesh1.Indices32.size();
    box1Submesh.StartIndexLocation = totalIndexCount;
    box1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box1Submesh.IndexCount;
    totalVertexCount += boxMesh1.Vertices.size();
    SubmeshGeometry box2Submesh;
    box2Submesh.IndexCount = (UINT)boxMesh2.Indices32.size();
    box2Submesh.StartIndexLocation = totalIndexCount;
    box2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box2Submesh.IndexCount;
    totalVertexCount += boxMesh2.Vertices.size();
    SubmeshGeometry box3Submesh;
    box3Submesh.IndexCount = (UINT)boxMesh3.Indices32.size();
    box3Submesh.StartIndexLocation = totalIndexCount;
    box3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box3Submesh.IndexCount;
    totalVertexCount += boxMesh3.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < boxMesh1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh1.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh1.Vertices.size();
    for (size_t i = 0; i < boxMesh2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh2.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh2.Vertices.size();
    for (size_t i = 0; i < boxMesh3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh3.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh3.Vertices.size();

    indices.insert(indices.end(), std::begin(boxMesh1.GetIndices16()), std::end(boxMesh1.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxMesh2.GetIndices16()), std::end(boxMesh2.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxMesh3.GetIndices16()), std::end(boxMesh3.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene7Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["box1"] = box1Submesh;
    geo->DrawArgs["box2"] = box2Submesh;
    geo->DrawArgs["box3"] = box3Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene9Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh1 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh2 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh3 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh4 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh5 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphere1Submesh;
    sphere1Submesh.IndexCount = (UINT)sphereMesh1.Indices32.size();
    sphere1Submesh.StartIndexLocation = totalIndexCount;
    sphere1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere1Submesh.IndexCount;
    totalVertexCount += sphereMesh1.Vertices.size();
    SubmeshGeometry sphere2Submesh;
    sphere2Submesh.IndexCount = (UINT)sphereMesh2.Indices32.size();
    sphere2Submesh.StartIndexLocation = totalIndexCount;
    sphere2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere2Submesh.IndexCount;
    totalVertexCount += sphereMesh2.Vertices.size();
    SubmeshGeometry sphere3Submesh;
    sphere3Submesh.IndexCount = (UINT)sphereMesh3.Indices32.size();
    sphere3Submesh.StartIndexLocation = totalIndexCount;
    sphere3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere3Submesh.IndexCount;
    totalVertexCount += sphereMesh3.Vertices.size();
    SubmeshGeometry sphere4Submesh;
    sphere4Submesh.IndexCount = (UINT)sphereMesh4.Indices32.size();
    sphere4Submesh.StartIndexLocation = totalIndexCount;
    sphere4Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere4Submesh.IndexCount;
    totalVertexCount += sphereMesh4.Vertices.size();
    SubmeshGeometry sphere5Submesh;
    sphere5Submesh.IndexCount = (UINT)sphereMesh5.Indices32.size();
    sphere5Submesh.StartIndexLocation = totalIndexCount;
    sphere5Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere5Submesh.IndexCount;
    totalVertexCount += sphereMesh5.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh1.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh1.Vertices.size();
    for (size_t i = 0; i < sphereMesh2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh2.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh2.Vertices.size();
    for (size_t i = 0; i < sphereMesh3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh3.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh3.Vertices.size();
    for (size_t i = 0; i < sphereMesh4.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh4.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh4.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh4.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh4.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh4.Vertices.size();
    for (size_t i = 0; i < sphereMesh5.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh5.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh5.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh5.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh5.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh5.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh1.GetIndices16()), std::end(sphereMesh1.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh2.GetIndices16()), std::end(sphereMesh2.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh3.GetIndices16()), std::end(sphereMesh3.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh4.GetIndices16()), std::end(sphereMesh4.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh5.GetIndices16()), std::end(sphereMesh5.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene9Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere1"] = sphere1Submesh;
    geo->DrawArgs["sphere2"] = sphere2Submesh;
    geo->DrawArgs["sphere3"] = sphere3Submesh;
    geo->DrawArgs["sphere4"] = sphere4Submesh;
    geo->DrawArgs["sphere5"] = sphere5Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene9RMDemoGeometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereSubmesh;
    sphereSubmesh.IndexCount = (UINT)sphereMesh.Indices32.size();
    sphereSubmesh.StartIndexLocation = totalIndexCount;
    sphereSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereSubmesh.IndexCount;
    totalVertexCount += sphereMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh.GetIndices16()), std::end(sphereMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene9RMDemoGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere"] = sphereSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildModelsGeometry()
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    ModelLoader::LoadModel("Obj/bonfire.obj", vertices, indices);
    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].Pos = DirectX::XMFLOAT3(vertices[i].Pos.x / 10 + 10.8f, vertices[i].Pos.y / 10, vertices[i].Pos.z / 10 + 1.f);
    }

    SubmeshGeometry bonfireSubmesh;
    bonfireSubmesh.IndexCount = indices.size();
    bonfireSubmesh.StartIndexLocation = 0;
    bonfireSubmesh.BaseVertexLocation = 0;
    bonfireSubmesh.VertexCount = vertices.size();

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene6Bonfire";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["Bonfire"] = bonfireSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene10DebugGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.0f, 7, 7, 0.f, 0.f, 0.f);
    GeometryGenerator::MeshData cone = geoGen.CreateCone(1.0f, 0.2f, 20);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry pointLight1Submesh;
    pointLight1Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight1Submesh.StartIndexLocation = totalIndexCount;
    pointLight1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight1Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight2Submesh;
    pointLight2Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight2Submesh.StartIndexLocation = totalIndexCount;
    pointLight2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight2Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight3Submesh;
    pointLight3Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight3Submesh.StartIndexLocation = totalIndexCount;
    pointLight3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight3Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry coneSubmesh;
    coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
    coneSubmesh.StartIndexLocation = totalIndexCount;
    coneSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += coneSubmesh.IndexCount;
    totalVertexCount += cone.Vertices.size();

    std::vector<DebugVertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;

    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < cone.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = cone.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += cone.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "debugGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["pointLight1"] = pointLight1Submesh;
    geo->DrawArgs["pointLight2"] = pointLight2Submesh;
    geo->DrawArgs["pointLight3"] = pointLight3Submesh;
    geo->DrawArgs["cone"] = coneSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene10MoreLightGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.f, 7, 7, 0.f, 0.f, 0.f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry pointLightSubmesh;
    pointLightSubmesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLightSubmesh.StartIndexLocation = totalIndexCount;
    pointLightSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLightSubmesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();


    std::vector<DebugVertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;

    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "moreLightGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["pointLight"] = pointLightSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene12Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData planeMesh = geoGen.CreateGrid(100, 100, 50, 50);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry planeSubmesh;
    planeSubmesh.IndexCount = (UINT)planeMesh.Indices32.size();
    planeSubmesh.StartIndexLocation = totalIndexCount;
    planeSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += planeSubmesh.IndexCount;
    totalVertexCount += planeMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < planeMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = planeMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = planeMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = planeMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = planeMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += planeMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(planeMesh.GetIndices16()), std::end(planeMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene12Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["plane"] = planeSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene13Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh = geoGen.CreateSphere(1.0f, 250, 250, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereSubmesh;
    sphereSubmesh.IndexCount = (UINT)sphereMesh.Indices32.size();
    sphereSubmesh.StartIndexLocation = totalIndexCount;
    sphereSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereSubmesh.IndexCount;
    totalVertexCount += sphereMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh.GetIndices16()), std::end(sphereMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene13Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere"] = sphereSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);

    BuildScene13InstanceBuffer();
    BuildScene13OctreeGeometry();
}

void Engine::BuildScene13InstanceBuffer()
{
    int instanceCount = 150;
    std::vector<InstanceDataGameObject> instanceData(instanceCount);
    for (UINT i = 0; i < instanceCount; ++i)
    {
        float xTranslation = std::rand() % 40 - 20;
        float yTranslation = std::rand() % 40 - 20;
        float zTranslation = std::rand() % 40 - 20;

        DirectX::XMStoreFloat4x4(&instanceData[i].WorldMatrix,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(xTranslation, yTranslation, zTranslation)));
        instanceData[i].Color = DirectX::XMFLOAT4(sin(i), cos(i), sin(2 * i), 1.0f);

        GameObject* gameObject = new GameObject();
        DirectX::BoundingBox boundingBox;
        boundingBox.Center = DirectX::XMFLOAT3(xTranslation, yTranslation, zTranslation);
        boundingBox.Extents = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
        gameObject->boundingBox = boundingBox;
        gameObject->objectID = i;
        gameObjects.push_back(gameObject);
    }
    instancesDataOcTree = instanceData;

    const UINT instanceBufferSize = instanceCount * sizeof(InstanceDataGameObject);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    md3dDevice->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&instanceOcTreeBuffer));

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    md3dDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceOcTreeUploadBuffer));

    D3D12_SUBRESOURCE_DATA instanceDataSub =
    {
        .pData = instanceData.data(),
        .RowPitch = instanceBufferSize,
        .SlicePitch = instanceBufferSize
    };

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceOcTreeBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    mCommandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(mCommandList.Get(),
        instanceOcTreeBuffer.Get(),
        instanceOcTreeUploadBuffer.Get(),
        0, 0, 1, &instanceDataSub);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceOcTreeBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    mCommandList->ResourceBarrier(1, &barrier2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = 150;
    srvDesc.Buffer.StructureByteStride = sizeof(InstanceDataGameObject);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(57, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(
        instanceOcTreeBuffer.Get(),
        &srvDesc,
        hDescriptor);

    DirectX::BoundingBox worldBoundingBox;
    worldBoundingBox.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    worldBoundingBox.Extents = DirectX::XMFLOAT3(20.0f, 20.0f, 20.0f);
    octreeScene13 = new Octree(md3dDevice.Get(), worldBoundingBox);
    octreeScene13->Build(gameObjects);
}

void Engine::BuildScene13OctreeGeometry()
{
    std::vector<DebugVertexOcTree> vertices;
    std::vector<std::uint16_t> indices;

    octreeScene13->CollectDebugNodes(octreeScene13->GetRoot(), vertices, indices);

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertexOcTree);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene13OctreeGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertexOcTree);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry octreeSubmesh;
    octreeSubmesh.IndexCount = indices.size();
    octreeSubmesh.StartIndexLocation = 0;
    octreeSubmesh.BaseVertexLocation = 0;

    geo->DrawArgs["octree"] = octreeSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene14Geometry()
{
   
}

void Engine::BuildQuadTreeTerrain()
{
    m_terrainQuadTree.Initialize(md3dDevice.Get(), 6, GeometryManager::mGeometries, mCommandList);
}

void Engine::BuildScene15Geometry()
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    ModelLoader::LoadModel("Obj/hamburger.obj", vertices, indices);
    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].Pos = DirectX::XMFLOAT3(vertices[i].Pos.x * 10.f, vertices[i].Pos.y * 10.f, vertices[i].Pos.z * 10.f);
    }

    SubmeshGeometry hamburgerSubmesh;
    hamburgerSubmesh.IndexCount = indices.size();
    hamburgerSubmesh.StartIndexLocation = 0;
    hamburgerSubmesh.BaseVertexLocation = 0;
    hamburgerSubmesh.VertexCount = vertices.size();

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene15Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["Hamburger"] = hamburgerSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildSponzaGeometryAndTextures()
{
    LoadModel("Sponza/sponza.obj", Sponza, md3dDevice.Get(),
        GeometryManager::mGeometries, mCommandList, mSponzaSrvHeap, mCbvSrvDescriptorSize,
        sponzaTextures, sponzaTexturesUpload);
}


void Engine::BuildPSOs()
{
    PSOManager::BuildPSOs(mRootSignatures, mPSOs, mInputLayouts, mShaders, mBackBufferFormat, mDepthStencilFormat, md3dDevice);
}

void Engine::BuildPostProcessingResources()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&postProcessingBuffer));


    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(7, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(postProcessingBuffer.Get(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(20, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(postProcessingBuffer.Get(), &srvDRDesc, hDescriptor);
}

void Engine::CreateNoiseTexture()
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.Width = WINDOW_WIDTH;
    textureDesc.Height = WINDOW_HEIGHT;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&noiseTexture)));

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = textureDesc.Format;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        mSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        21,
        mCbvSrvDescriptorSize);

    md3dDevice->CreateUnorderedAccessView(noiseTexture.Get(), nullptr, &uavDesc, handle);

    handle.Offset(1, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = noiseTexture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    md3dDevice->CreateShaderResourceView(noiseTexture.Get(), &srvDesc, handle);
}

void Engine::BuildFrameResources()
{
    for (int i = 0; i < gNumFrameResources; ++i)
    {
        CBManager::mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            2, (UINT)GeometryManager::mAllRitems.size(), (UINT)GeometryManager::mMaterials.size()));
    }
}

void Engine::BuildMaterials()
{
    auto metalAnimate = std::make_unique<Material>();
    metalAnimate->Name = "metalAnimate";
    metalAnimate->MatCBIndex = 0;
    metalAnimate->DiffuseSrvHeapIndex = 1;
    metalAnimate->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    metalAnimate->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
    metalAnimate->Roughness = 0.2f;

    GeometryManager::mMaterials["metalAnimate"] = std::move(metalAnimate);

    auto tileCrate = std::make_unique<Material>();
    tileCrate->Name = "tileCrate";
    tileCrate->MatCBIndex = 1;
    tileCrate->DiffuseSrvHeapIndex = 1;
    tileCrate->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    tileCrate->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
    tileCrate->Roughness = 0.2f;

    GeometryManager::mMaterials["tileCrate"] = std::move(tileCrate);

    auto stoneMat = std::make_unique<Material>();
    stoneMat->Name = "stoneMaterial";
    stoneMat->MatCBIndex = 2;
    stoneMat->DiffuseSrvHeapIndex = 6;
    stoneMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    stoneMat->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    stoneMat->Roughness = 0.5f;

    GeometryManager::mMaterials["stoneMaterial"] = std::move(stoneMat);

    auto planetMat = std::make_unique<Material>();
    planetMat->Name = "planetMaterial";
    planetMat->MatCBIndex = 3;
    planetMat->DiffuseSrvHeapIndex = 17;
    planetMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    planetMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    planetMat->Roughness = 0.125f;

    GeometryManager::mMaterials["planetMaterial"] = std::move(planetMat);

    auto particleMat = std::make_unique<Material>();
    particleMat->Name = "particleMaterial";
    particleMat->MatCBIndex = 4;
    particleMat->DiffuseSrvHeapIndex = 21;
    particleMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particleMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particleMat->Roughness = 0.125f;

    GeometryManager::mMaterials["particleMaterial"] = std::move(particleMat);

    auto particle2Mat = std::make_unique<Material>();
    particle2Mat->Name = "particle2Material";
    particle2Mat->MatCBIndex = 4;
    particle2Mat->DiffuseSrvHeapIndex = 22;
    particle2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particle2Mat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particle2Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["particle2Material"] = std::move(particle2Mat);

    auto skyboxMat = std::make_unique<Material>();
    skyboxMat->Name = "skyboxMaterial";
    skyboxMat->MatCBIndex = 5;
    skyboxMat->DiffuseSrvHeapIndex = 18;
    skyboxMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    skyboxMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    skyboxMat->Roughness = 0.125f;

    GeometryManager::mMaterials["skyboxMaterial"] = std::move(skyboxMat);

    auto smokeMat = std::make_unique<Material>();
    smokeMat->Name = "smokeMaterial";
    smokeMat->MatCBIndex = 6;
    smokeMat->DiffuseSrvHeapIndex = 23;
    smokeMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    smokeMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    smokeMat->Roughness = 0.125f;

    GeometryManager::mMaterials["smokeMaterial"] = std::move(smokeMat);

    auto bonfireMat = std::make_unique<Material>();
    bonfireMat->Name = "bonfireMaterial";
    bonfireMat->MatCBIndex = 7;
    bonfireMat->DiffuseSrvHeapIndex = 24;
    bonfireMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    bonfireMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    bonfireMat->Roughness = 0.125f;

    GeometryManager::mMaterials["bonfireMaterial"] = std::move(bonfireMat);

    auto grassMat = std::make_unique<Material>();
    grassMat->Name = "grassMaterial";
    grassMat->MatCBIndex = 8;
    grassMat->DiffuseSrvHeapIndex = 25;
    grassMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    grassMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    grassMat->Roughness = 0.125f;

    GeometryManager::mMaterials["grassMaterial"] = std::move(grassMat);

    auto PBR1Mat = std::make_unique<Material>();
    PBR1Mat->Name = "PBR1Material";
    PBR1Mat->MatCBIndex = 9;
    PBR1Mat->DiffuseSrvHeapIndex = 23;
    PBR1Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR1Mat->FresnelR0 = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    PBR1Mat->Roughness = 0.125f;
    PBR1Mat->IsMetallic = 1;

    GeometryManager::mMaterials["PBR1Material"] = std::move(PBR1Mat);

    auto PBR2Mat = std::make_unique<Material>();
    PBR2Mat->Name = "PBR2Material";
    PBR2Mat->MatCBIndex = 10;
    PBR2Mat->DiffuseSrvHeapIndex = 28;
    PBR2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR2Mat->FresnelR0 = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    PBR2Mat->Roughness = 0.125f;
    PBR2Mat->IsMetallic = 1;

    GeometryManager::mMaterials["PBR2Material"] = std::move(PBR2Mat);

    auto PBR3Mat = std::make_unique<Material>();
    PBR3Mat->Name = "PBR3Material";
    PBR3Mat->MatCBIndex = 11;
    PBR3Mat->DiffuseSrvHeapIndex = 33;
    PBR3Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR3Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR3Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR3Material"] = std::move(PBR3Mat);

    auto PBR4Mat = std::make_unique<Material>();
    PBR4Mat->Name = "PBR4Material";
    PBR4Mat->MatCBIndex = 12;
    PBR4Mat->DiffuseSrvHeapIndex = 38;
    PBR4Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR4Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR4Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR4Material"] = std::move(PBR4Mat);

    auto PBR5Mat = std::make_unique<Material>();
    PBR5Mat->Name = "PBR5Material";
    PBR5Mat->MatCBIndex = 13;
    PBR5Mat->DiffuseSrvHeapIndex = 43;
    PBR5Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR5Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR5Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR5Material"] = std::move(PBR5Mat);

    auto TerrainMat = std::make_unique<Material>();
    TerrainMat->Name = "TerrainMaterial";
    TerrainMat->MatCBIndex = 14;
    TerrainMat->DiffuseSrvHeapIndex = 54;
    TerrainMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    TerrainMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    TerrainMat->Roughness = 0.9f;

    GeometryManager::mMaterials["TerrainMaterial"] = std::move(TerrainMat);

    auto RainMat = std::make_unique<Material>();
    RainMat->Name = "RainMaterial";
    RainMat->MatCBIndex = 15;
    RainMat->DiffuseSrvHeapIndex = 27;
    RainMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    RainMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    RainMat->Roughness = 0.9f;

    GeometryManager::mMaterials["RainMaterial"] = std::move(RainMat);

    auto Terrain2Mat = std::make_unique<Material>();
    Terrain2Mat->Name = "TerrainMaterial2";
    Terrain2Mat->MatCBIndex = 16;
    Terrain2Mat->DiffuseSrvHeapIndex = 59;
    Terrain2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    Terrain2Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    Terrain2Mat->Roughness = 0.9f;

    GeometryManager::mMaterials["TerrainMaterial2"] = std::move(Terrain2Mat);

    auto HamburgerMat = std::make_unique<Material>();
    HamburgerMat->Name = "HamburgerMaterial";
    HamburgerMat->MatCBIndex = 17;
    HamburgerMat->DiffuseSrvHeapIndex = 62;
    HamburgerMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    HamburgerMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    HamburgerMat->Roughness = 0.9f;

    GeometryManager::mMaterials["HamburgerMaterial"] = std::move(HamburgerMat);
}

void Engine::BuildRenderItems()
{
    auto boxRitem = std::make_unique<RenderItem>();
    boxRitem->ObjCBIndex = 0;
    boxRitem->World = MathHelper::Identity4x4();
    boxRitem->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    boxRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxRitem));

    auto boxTileRitem = std::make_unique<RenderItem>();
    boxTileRitem->ObjCBIndex = 1;
    XMStoreFloat4x4(&boxTileRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(2.0f, 2.0f, 0.0f));
    boxTileRitem->Mat = GeometryManager::mMaterials["tileCrate"].get();
    boxTileRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxTileRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxTileRitem->IndexCount = boxTileRitem->Geo->DrawArgs["box2"].IndexCount;
    boxTileRitem->StartIndexLocation = boxTileRitem->Geo->DrawArgs["box2"].StartIndexLocation;
    boxTileRitem->BaseVertexLocation = boxTileRitem->Geo->DrawArgs["box2"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxTileRitem));


    auto stoneTessellationRitem = std::make_unique<RenderItem>();
    stoneTessellationRitem->ObjCBIndex = 2;
    XMStoreFloat4x4(&stoneTessellationRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f));
    stoneTessellationRitem->TexTransform = MathHelper::Identity4x4();
    stoneTessellationRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    stoneTessellationRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    stoneTessellationRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    stoneTessellationRitem->IndexCount = stoneTessellationRitem->Geo->DrawArgs["box3"].IndexCount;
    stoneTessellationRitem->StartIndexLocation = stoneTessellationRitem->Geo->DrawArgs["box3"].StartIndexLocation;
    stoneTessellationRitem->BaseVertexLocation = stoneTessellationRitem->Geo->DrawArgs["box3"].BaseVertexLocation;
    mRitemLayer[(int)RenderLayer::Scene11].push_back(stoneTessellationRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(stoneTessellationRitem));


    auto boxInstancingRitem = std::make_unique<RenderItem>();
    boxInstancingRitem->ObjCBIndex = 3;
    boxInstancingRitem->World = MathHelper::Identity4x4();
    boxInstancingRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    boxInstancingRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxInstancingRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxInstancingRitem->IndexCount = boxInstancingRitem->Geo->DrawArgs["box4"].IndexCount;
    boxInstancingRitem->StartIndexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].StartIndexLocation;
    boxInstancingRitem->BaseVertexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxInstancingRitem));

    auto boxFPSRitem = std::make_unique<RenderItem>();
    boxFPSRitem->ObjCBIndex = 4;
    boxFPSRitem->World = MathHelper::Identity4x4();
    boxFPSRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    boxFPSRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxFPSRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxFPSRitem->IndexCount = boxFPSRitem->Geo->DrawArgs["boxFPS"].IndexCount;
    boxFPSRitem->StartIndexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].StartIndexLocation;
    boxFPSRitem->BaseVertexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxFPSRitem));

    auto pointLight1Ritem = std::make_unique<RenderItem>();
    pointLight1Ritem->ObjCBIndex = 5;
    pointLight1Ritem->World = MathHelper::Identity4x4();
    pointLight1Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight1Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight1Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight1Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight1Ritem->IndexCount = pointLight1Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    pointLight1Ritem->StartIndexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    pointLight1Ritem->BaseVertexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight1Ritem));

    auto pointLight2Ritem = std::make_unique<RenderItem>();
    pointLight2Ritem->ObjCBIndex = 6;
    pointLight2Ritem->World = MathHelper::Identity4x4();
    pointLight2Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight2Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight2Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight2Ritem->IndexCount = pointLight2Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    pointLight2Ritem->StartIndexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    pointLight2Ritem->BaseVertexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight2Ritem));

    auto pointLight3Ritem = std::make_unique<RenderItem>();
    pointLight3Ritem->ObjCBIndex = 7;
    pointLight3Ritem->World = MathHelper::Identity4x4();
    pointLight3Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight3Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight3Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight3Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight3Ritem->IndexCount = pointLight3Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    pointLight3Ritem->StartIndexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    pointLight3Ritem->BaseVertexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight3Ritem));

    auto gridRitem = std::make_unique<RenderItem>();
    gridRitem->ObjCBIndex = 8;
    gridRitem->World = MathHelper::Identity4x4();
    gridRitem->TexTransform = MathHelper::Identity4x4();
    gridRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    gridRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(gridRitem));

    auto coneRitem = std::make_unique<RenderItem>();
    coneRitem->ObjCBIndex = 9;
    coneRitem->World = MathHelper::Identity4x4();
    coneRitem->TexTransform = MathHelper::Identity4x4();
    coneRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    coneRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    coneRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    coneRitem->IndexCount = coneRitem->Geo->DrawArgs["cone"].IndexCount;
    coneRitem->StartIndexLocation = coneRitem->Geo->DrawArgs["cone"].StartIndexLocation;
    coneRitem->BaseVertexLocation = coneRitem->Geo->DrawArgs["cone"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(coneRitem));

    auto planetSpritesRitem = std::make_unique<RenderItem>();
    planetSpritesRitem->World = MathHelper::Identity4x4();
    planetSpritesRitem->ObjCBIndex = 10;
    planetSpritesRitem->Mat = GeometryManager::mMaterials["planetMaterial"].get();
    planetSpritesRitem->Geo = GeometryManager::mGeometries["planetSpritesGeo"].get();
    planetSpritesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    planetSpritesRitem->IndexCount = planetSpritesRitem->Geo->DrawArgs["points"].IndexCount;
    planetSpritesRitem->StartIndexLocation = planetSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
    planetSpritesRitem->BaseVertexLocation = planetSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::BillboardSprites].push_back(planetSpritesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(planetSpritesRitem));

    auto particleRitem = std::make_unique<RenderItem>();
    particleRitem->World = MathHelper::Identity4x4();
    particleRitem->ObjCBIndex = 11;
    particleRitem->Mat = GeometryManager::mMaterials["particleMaterial"].get();
    particleRitem->Geo = GeometryManager::mGeometries["particlesGeo1"].get();
    particleRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particleRitem->IndexCount = particleRitem->Geo->DrawArgs["points1"].IndexCount;
    particleRitem->StartIndexLocation = particleRitem->Geo->DrawArgs["points1"].StartIndexLocation;
    particleRitem->BaseVertexLocation = particleRitem->Geo->DrawArgs["points1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles1].push_back(particleRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(particleRitem));

    auto skyboxRitem = std::make_unique<RenderItem>();
    skyboxRitem->World = MathHelper::Identity4x4();
    skyboxRitem->ObjCBIndex = 12;
    skyboxRitem->Mat = GeometryManager::mMaterials["skyboxMaterial"].get();
    skyboxRitem->Geo = GeometryManager::mGeometries["skyboxGeo"].get();
    skyboxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    skyboxRitem->IndexCount = skyboxRitem->Geo->DrawArgs["skybox"].IndexCount;
    skyboxRitem->StartIndexLocation = skyboxRitem->Geo->DrawArgs["skybox"].StartIndexLocation;
    skyboxRitem->BaseVertexLocation = skyboxRitem->Geo->DrawArgs["skybox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Sky].push_back(skyboxRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(skyboxRitem));

    auto instancingRitemScene3 = std::make_unique<RenderItem>();
    instancingRitemScene3->World = MathHelper::Identity4x4();
    instancingRitemScene3->ObjCBIndex = 13;
    instancingRitemScene3->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    instancingRitemScene3->Geo = GeometryManager::mGeometries["scene3Geo"].get();
    instancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    instancingRitemScene3->IndexCount = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
    instancingRitemScene3->StartIndexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
    instancingRitemScene3->BaseVertexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;
    instancingRitemScene3->Bounds = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].Bounds;

    mRitemLayer[(int)RenderLayer::Scene3].push_back(instancingRitemScene3.get());
    GeometryManager::mAllRitems.push_back(std::move(instancingRitemScene3));

    for (int i = 0; i < 729; ++i)
    {
        auto nonInstancingRitemScene3 = std::make_unique<RenderItem>();
        nonInstancingRitemScene3->World = MathHelper::Identity4x4();
        nonInstancingRitemScene3->ObjCBIndex = 14 + i;
        nonInstancingRitemScene3->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
        nonInstancingRitemScene3->Geo = GeometryManager::mGeometries["scene3Geo"].get();
        nonInstancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        nonInstancingRitemScene3->IndexCount = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
        nonInstancingRitemScene3->StartIndexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
        nonInstancingRitemScene3->BaseVertexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;

        mRitemLayer[(int)RenderLayer::Scene3].push_back(nonInstancingRitemScene3.get());
        GeometryManager::mAllRitems.push_back(std::move(nonInstancingRitemScene3));
    }

    auto animateRitemScene4 = std::make_unique<RenderItem>();
    animateRitemScene4->World = MathHelper::Identity4x4();
    animateRitemScene4->ObjCBIndex = 743;
    animateRitemScene4->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    animateRitemScene4->Geo = GeometryManager::mGeometries["scene4Geo"].get();
    animateRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    animateRitemScene4->IndexCount = animateRitemScene4->Geo->DrawArgs["animateBox"].IndexCount;
    animateRitemScene4->StartIndexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].StartIndexLocation;
    animateRitemScene4->BaseVertexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(animateRitemScene4.get());
    GeometryManager::mAllRitems.push_back(std::move(animateRitemScene4));

    auto tilingRitemScene4 = std::make_unique<RenderItem>();
    tilingRitemScene4->World = MathHelper::Identity4x4();
    tilingRitemScene4->ObjCBIndex = 744;
    tilingRitemScene4->Mat = GeometryManager::mMaterials["tileCrate"].get();
    tilingRitemScene4->Geo = GeometryManager::mGeometries["scene4Geo"].get();
    tilingRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tilingRitemScene4->IndexCount = tilingRitemScene4->Geo->DrawArgs["tilingBox"].IndexCount;
    tilingRitemScene4->StartIndexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].StartIndexLocation;
    tilingRitemScene4->BaseVertexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(tilingRitemScene4.get());
    GeometryManager::mAllRitems.push_back(std::move(tilingRitemScene4));

    auto tessellationRitemScene5 = std::make_unique<RenderItem>();
    tessellationRitemScene5->World = MathHelper::Identity4x4();
    tessellationRitemScene5->ObjCBIndex = 745;
    tessellationRitemScene5->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    tessellationRitemScene5->Geo = GeometryManager::mGeometries["scene5Geo"].get();
    tessellationRitemScene5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tessellationRitemScene5->IndexCount = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].IndexCount;
    tessellationRitemScene5->StartIndexLocation = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].StartIndexLocation;
    tessellationRitemScene5->BaseVertexLocation = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene5].push_back(tessellationRitemScene5.get());
    GeometryManager::mAllRitems.push_back(std::move(tessellationRitemScene5));

    auto particle2Ritem = std::make_unique<RenderItem>();
    particle2Ritem->World = MathHelper::Identity4x4();
    particle2Ritem->ObjCBIndex = 746;
    particle2Ritem->Mat = GeometryManager::mMaterials["particle2Material"].get();
    particle2Ritem->Geo = GeometryManager::mGeometries["particlesGeo2"].get();
    particle2Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particle2Ritem->IndexCount = particle2Ritem->Geo->DrawArgs["points2"].IndexCount;
    particle2Ritem->StartIndexLocation = particle2Ritem->Geo->DrawArgs["points2"].StartIndexLocation;
    particle2Ritem->BaseVertexLocation = particle2Ritem->Geo->DrawArgs["points2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles2].push_back(particle2Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(particle2Ritem));

    auto sponzaRitem = std::make_unique<RenderItem>();
    sponzaRitem->World = MathHelper::Identity4x4();
    sponzaRitem->ObjCBIndex = 747;
    sponzaRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    sponzaRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    GeometryManager::mAllRitems.push_back(std::move(sponzaRitem));

    auto bonfireRitem = std::make_unique<RenderItem>();
    bonfireRitem->World = MathHelper::Identity4x4();
    bonfireRitem->ObjCBIndex = 748;
    bonfireRitem->Mat = GeometryManager::mMaterials["bonfireMaterial"].get();
    bonfireRitem->Geo = GeometryManager::mGeometries["scene6Bonfire"].get();
    bonfireRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    bonfireRitem->IndexCount = bonfireRitem->Geo->DrawArgs["Bonfire"].IndexCount;
    bonfireRitem->StartIndexLocation = bonfireRitem->Geo->DrawArgs["Bonfire"].StartIndexLocation;
    bonfireRitem->BaseVertexLocation = bonfireRitem->Geo->DrawArgs["Bonfire"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene6_3].push_back(bonfireRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(bonfireRitem));

    auto grassPlatformRitem = std::make_unique<RenderItem>();
    grassPlatformRitem->World = MathHelper::Identity4x4();
    grassPlatformRitem->ObjCBIndex = 749;
    grassPlatformRitem->Mat = GeometryManager::mMaterials["grassMaterial"].get();
    grassPlatformRitem->Geo = GeometryManager::mGeometries["scene6GrassPlatform"].get();
    grassPlatformRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    grassPlatformRitem->IndexCount = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].IndexCount;
    grassPlatformRitem->StartIndexLocation = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].StartIndexLocation;
    grassPlatformRitem->BaseVertexLocation = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene6_3].push_back(grassPlatformRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(grassPlatformRitem));

    auto particleSmokeRitem = std::make_unique<RenderItem>();
    particleSmokeRitem->World = MathHelper::Identity4x4();
    particleSmokeRitem->ObjCBIndex = 750;
    particleSmokeRitem->Mat = GeometryManager::mMaterials["smokeMaterial"].get();
    particleSmokeRitem->Geo = GeometryManager::mGeometries["particlesGeo3"].get();
    particleSmokeRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particleSmokeRitem->IndexCount = particleSmokeRitem->Geo->DrawArgs["points3"].IndexCount;
    particleSmokeRitem->StartIndexLocation = particleSmokeRitem->Geo->DrawArgs["points3"].StartIndexLocation;
    particleSmokeRitem->BaseVertexLocation = particleSmokeRitem->Geo->DrawArgs["points3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles3].push_back(particleSmokeRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(particleSmokeRitem));

    auto pointLight1Scene10Ritem = std::make_unique<RenderItem>();
    pointLight1Scene10Ritem->ObjCBIndex = 751;
    pointLight1Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight1Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight1Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight1Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight1Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight1Scene10Ritem->IndexCount = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    pointLight1Scene10Ritem->StartIndexLocation = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    pointLight1Scene10Ritem->BaseVertexLocation = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight1Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight1Scene10Ritem));

    auto pointLight2Scene10Ritem = std::make_unique<RenderItem>();
    pointLight2Scene10Ritem->ObjCBIndex = 752;
    pointLight2Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight2Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight2Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight2Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight2Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight2Scene10Ritem->IndexCount = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    pointLight2Scene10Ritem->StartIndexLocation = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    pointLight2Scene10Ritem->BaseVertexLocation = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight2Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight2Scene10Ritem));

    auto pointLight3Scene10Ritem = std::make_unique<RenderItem>();
    pointLight3Scene10Ritem->ObjCBIndex = 753;
    pointLight3Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight3Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight3Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight3Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight3Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight3Scene10Ritem->IndexCount = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    pointLight3Scene10Ritem->StartIndexLocation = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    pointLight3Scene10Ritem->BaseVertexLocation = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight3Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight3Scene10Ritem));

    auto coneLightScene10Ritem = std::make_unique<RenderItem>();
    coneLightScene10Ritem->ObjCBIndex = 754;
    coneLightScene10Ritem->World = MathHelper::Identity4x4();
    coneLightScene10Ritem->TexTransform = MathHelper::Identity4x4();
    coneLightScene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    coneLightScene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    coneLightScene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    coneLightScene10Ritem->IndexCount = coneLightScene10Ritem->Geo->DrawArgs["cone"].IndexCount;
    coneLightScene10Ritem->StartIndexLocation = coneLightScene10Ritem->Geo->DrawArgs["cone"].StartIndexLocation;
    coneLightScene10Ritem->BaseVertexLocation = coneLightScene10Ritem->Geo->DrawArgs["cone"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(coneLightScene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(coneLightScene10Ritem));

    auto box1Scene7Ritem = std::make_unique<RenderItem>();
    box1Scene7Ritem->ObjCBIndex = 755;
    box1Scene7Ritem->World = MathHelper::Identity4x4();
    box1Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box1Scene7Ritem->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    box1Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box1Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box1Scene7Ritem->IndexCount = box1Scene7Ritem->Geo->DrawArgs["box1"].IndexCount;
    box1Scene7Ritem->StartIndexLocation = box1Scene7Ritem->Geo->DrawArgs["box1"].StartIndexLocation;
    box1Scene7Ritem->BaseVertexLocation = box1Scene7Ritem->Geo->DrawArgs["box1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box1Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box1Scene7Ritem));

    auto box2Scene7Ritem = std::make_unique<RenderItem>();
    box2Scene7Ritem->ObjCBIndex = 756;
    box2Scene7Ritem->World = MathHelper::Identity4x4();
    box2Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box2Scene7Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    box2Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box2Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box2Scene7Ritem->IndexCount = box2Scene7Ritem->Geo->DrawArgs["box2"].IndexCount;
    box2Scene7Ritem->StartIndexLocation = box2Scene7Ritem->Geo->DrawArgs["box2"].StartIndexLocation;
    box2Scene7Ritem->BaseVertexLocation = box2Scene7Ritem->Geo->DrawArgs["box2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box2Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box2Scene7Ritem));

    auto box3Scene7Ritem = std::make_unique<RenderItem>();
    box3Scene7Ritem->ObjCBIndex = 757;
    box3Scene7Ritem->World = MathHelper::Identity4x4();
    box3Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box3Scene7Ritem->Mat = GeometryManager::mMaterials["planetMaterial"].get();
    box3Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box3Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box3Scene7Ritem->IndexCount = box3Scene7Ritem->Geo->DrawArgs["box3"].IndexCount;
    box3Scene7Ritem->StartIndexLocation = box3Scene7Ritem->Geo->DrawArgs["box3"].StartIndexLocation;
    box3Scene7Ritem->BaseVertexLocation = box3Scene7Ritem->Geo->DrawArgs["box3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box3Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box3Scene7Ritem));

    auto PBR1Scene9Ritem = std::make_unique<RenderItem>();
    PBR1Scene9Ritem->ObjCBIndex = 758;
    PBR1Scene9Ritem->World = MathHelper::Identity4x4();
    PBR1Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR1Scene9Ritem->Mat = GeometryManager::mMaterials["PBR1Material"].get();
    PBR1Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR1Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR1Scene9Ritem->IndexCount = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].IndexCount;
    PBR1Scene9Ritem->StartIndexLocation = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].StartIndexLocation;
    PBR1Scene9Ritem->BaseVertexLocation = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR1Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR1Scene9Ritem));

    auto PBR2Scene9Ritem = std::make_unique<RenderItem>();
    PBR2Scene9Ritem->ObjCBIndex = 759;
    PBR2Scene9Ritem->World = MathHelper::Identity4x4();
    PBR2Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR2Scene9Ritem->Mat = GeometryManager::mMaterials["PBR2Material"].get();
    PBR2Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR2Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR2Scene9Ritem->IndexCount = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].IndexCount;
    PBR2Scene9Ritem->StartIndexLocation = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].StartIndexLocation;
    PBR2Scene9Ritem->BaseVertexLocation = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR2Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR2Scene9Ritem));

    auto PBR3Scene9Ritem = std::make_unique<RenderItem>();
    PBR3Scene9Ritem->ObjCBIndex = 760;
    PBR3Scene9Ritem->World = MathHelper::Identity4x4();
    PBR3Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR3Scene9Ritem->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    PBR3Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR3Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR3Scene9Ritem->IndexCount = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].IndexCount;
    PBR3Scene9Ritem->StartIndexLocation = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].StartIndexLocation;
    PBR3Scene9Ritem->BaseVertexLocation = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR3Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR3Scene9Ritem));

    auto PBR4Scene9Ritem = std::make_unique<RenderItem>();
    PBR4Scene9Ritem->ObjCBIndex = 761;
    PBR4Scene9Ritem->World = MathHelper::Identity4x4();
    PBR4Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR4Scene9Ritem->Mat = GeometryManager::mMaterials["PBR4Material"].get();
    PBR4Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR4Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR4Scene9Ritem->IndexCount = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].IndexCount;
    PBR4Scene9Ritem->StartIndexLocation = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].StartIndexLocation;
    PBR4Scene9Ritem->BaseVertexLocation = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR4Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR4Scene9Ritem));

    auto PBR5Scene9Ritem = std::make_unique<RenderItem>();
    PBR5Scene9Ritem->ObjCBIndex = 762;
    PBR5Scene9Ritem->World = MathHelper::Identity4x4();
    PBR5Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR5Scene9Ritem->Mat = GeometryManager::mMaterials["PBR5Material"].get();
    PBR5Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR5Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR5Scene9Ritem->IndexCount = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].IndexCount;
    PBR5Scene9Ritem->StartIndexLocation = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].StartIndexLocation;
    PBR5Scene9Ritem->BaseVertexLocation = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR5Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR5Scene9Ritem));

    auto sphere0LOD = std::make_unique<RenderItem>();
    sphere0LOD->ObjCBIndex = 763;
    sphere0LOD->World = MathHelper::Identity4x4();
    sphere0LOD->TexTransform = MathHelper::Identity4x4();
    sphere0LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere0LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere0LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere0LOD->IndexCount = sphere0LOD->Geo->DrawArgs["sphereLOD0"].IndexCount;
    sphere0LOD->StartIndexLocation = sphere0LOD->Geo->DrawArgs["sphereLOD0"].StartIndexLocation;
    sphere0LOD->BaseVertexLocation = sphere0LOD->Geo->DrawArgs["sphereLOD0"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere0LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere0LOD));

    auto sphere1LOD = std::make_unique<RenderItem>();
    sphere1LOD->ObjCBIndex = 764;
    sphere1LOD->World = MathHelper::Identity4x4();
    sphere1LOD->TexTransform = MathHelper::Identity4x4();
    sphere1LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere1LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere1LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere1LOD->IndexCount = sphere1LOD->Geo->DrawArgs["sphereLOD1"].IndexCount;
    sphere1LOD->StartIndexLocation = sphere1LOD->Geo->DrawArgs["sphereLOD1"].StartIndexLocation;
    sphere1LOD->BaseVertexLocation = sphere1LOD->Geo->DrawArgs["sphereLOD1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere1LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere1LOD));

    auto sphere2LOD = std::make_unique<RenderItem>();
    sphere2LOD->ObjCBIndex = 765;
    sphere2LOD->World = MathHelper::Identity4x4();
    sphere2LOD->TexTransform = MathHelper::Identity4x4();
    sphere2LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere2LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere2LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere2LOD->IndexCount = sphere2LOD->Geo->DrawArgs["sphereLOD2"].IndexCount;
    sphere2LOD->StartIndexLocation = sphere2LOD->Geo->DrawArgs["sphereLOD2"].StartIndexLocation;
    sphere2LOD->BaseVertexLocation = sphere2LOD->Geo->DrawArgs["sphereLOD2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere2LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere2LOD));

    auto sphere3LOD = std::make_unique<RenderItem>();
    sphere3LOD->ObjCBIndex = 766;
    sphere3LOD->World = MathHelper::Identity4x4();
    sphere3LOD->TexTransform = MathHelper::Identity4x4();
    sphere3LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere3LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere3LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere3LOD->IndexCount = sphere3LOD->Geo->DrawArgs["sphereLOD3"].IndexCount;
    sphere3LOD->StartIndexLocation = sphere3LOD->Geo->DrawArgs["sphereLOD3"].StartIndexLocation;
    sphere3LOD->BaseVertexLocation = sphere3LOD->Geo->DrawArgs["sphereLOD3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere3LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere3LOD));

    auto sphereRMDemo = std::make_unique<RenderItem>();
    sphereRMDemo->ObjCBIndex = 767;
    sphereRMDemo->World = MathHelper::Identity4x4();
    sphereRMDemo->TexTransform = MathHelper::Identity4x4();
    sphereRMDemo->Mat = GeometryManager::mMaterials["PBR1Material"].get();
    sphereRMDemo->Geo = GeometryManager::mGeometries["scene9RMDemoGeo"].get();
    sphereRMDemo->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphereRMDemo->IndexCount = sphereRMDemo->Geo->DrawArgs["sphere"].IndexCount;
    sphereRMDemo->StartIndexLocation = sphereRMDemo->Geo->DrawArgs["sphere"].StartIndexLocation;
    sphereRMDemo->BaseVertexLocation = sphereRMDemo->Geo->DrawArgs["sphere"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9RMDemo].push_back(sphereRMDemo.get());
    GeometryManager::mAllRitems.push_back(std::move(sphereRMDemo));

    auto terrainRitem = std::make_unique<RenderItem>();
    terrainRitem->ObjCBIndex = 768;
    terrainRitem->World = MathHelper::Identity4x4();
    terrainRitem->TexTransform = MathHelper::Identity4x4();
    terrainRitem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    terrainRitem->Geo = GeometryManager::mGeometries["scene12Geo"].get();
    terrainRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    terrainRitem->IndexCount = terrainRitem->Geo->DrawArgs["plane"].IndexCount;
    terrainRitem->StartIndexLocation = terrainRitem->Geo->DrawArgs["plane"].StartIndexLocation;
    terrainRitem->BaseVertexLocation = terrainRitem->Geo->DrawArgs["plane"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene12].push_back(terrainRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(terrainRitem));

    auto moreLightRitem = std::make_unique<RenderItem>();
    moreLightRitem->ObjCBIndex = 769;
    moreLightRitem->World = MathHelper::Identity4x4();
    moreLightRitem->TexTransform = MathHelper::Identity4x4();
    moreLightRitem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    moreLightRitem->Geo = GeometryManager::mGeometries["moreLightGeo"].get();
    moreLightRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    moreLightRitem->IndexCount = moreLightRitem->Geo->DrawArgs["pointLight"].IndexCount;
    moreLightRitem->StartIndexLocation = moreLightRitem->Geo->DrawArgs["pointLight"].StartIndexLocation;
    moreLightRitem->BaseVertexLocation = moreLightRitem->Geo->DrawArgs["pointLight"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10MoreLight].push_back(moreLightRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(moreLightRitem));

    auto sphereScene13Ritem = std::make_unique<RenderItem>();
    sphereScene13Ritem->ObjCBIndex = 770;
    sphereScene13Ritem->World = MathHelper::Identity4x4();
    sphereScene13Ritem->TexTransform = MathHelper::Identity4x4();
    sphereScene13Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    sphereScene13Ritem->Geo = GeometryManager::mGeometries["scene13Geo"].get();
    sphereScene13Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphereScene13Ritem->IndexCount = sphereScene13Ritem->Geo->DrawArgs["sphere"].IndexCount;
    sphereScene13Ritem->StartIndexLocation = sphereScene13Ritem->Geo->DrawArgs["sphere"].StartIndexLocation;
    sphereScene13Ritem->BaseVertexLocation = sphereScene13Ritem->Geo->DrawArgs["sphere"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene13].push_back(sphereScene13Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(sphereScene13Ritem));

    auto octreeScene13Ritem = std::make_unique<RenderItem>();
    octreeScene13Ritem->ObjCBIndex = 771;
    octreeScene13Ritem->World = MathHelper::Identity4x4();
    octreeScene13Ritem->TexTransform = MathHelper::Identity4x4();
    octreeScene13Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    octreeScene13Ritem->Geo = GeometryManager::mGeometries["scene13OctreeGeo"].get();
    octreeScene13Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    octreeScene13Ritem->IndexCount = octreeScene13Ritem->Geo->DrawArgs["octree"].IndexCount;
    octreeScene13Ritem->StartIndexLocation = octreeScene13Ritem->Geo->DrawArgs["octree"].StartIndexLocation;
    octreeScene13Ritem->BaseVertexLocation = octreeScene13Ritem->Geo->DrawArgs["octree"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene13Octree].push_back(octreeScene13Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(octreeScene13Ritem));

    auto rainParticlesRitem = std::make_unique<RenderItem>();
    rainParticlesRitem->ObjCBIndex = 772;
    rainParticlesRitem->World = MathHelper::Identity4x4();
    rainParticlesRitem->TexTransform = MathHelper::Identity4x4();
    rainParticlesRitem->Mat = GeometryManager::mMaterials["RainMaterial"].get();
    rainParticlesRitem->Geo = GeometryManager::mGeometries["particlesGeo4"].get();
    rainParticlesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    rainParticlesRitem->IndexCount = rainParticlesRitem->Geo->DrawArgs["points4"].IndexCount;
    rainParticlesRitem->StartIndexLocation = rainParticlesRitem->Geo->DrawArgs["points4"].StartIndexLocation;
    rainParticlesRitem->BaseVertexLocation = rainParticlesRitem->Geo->DrawArgs["points4"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::RainParticles].push_back(rainParticlesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(rainParticlesRitem));

    auto terrain2Ritem = std::make_unique<RenderItem>();
    terrain2Ritem->ObjCBIndex = 773;
    terrain2Ritem->World = MathHelper::Identity4x4();
    terrain2Ritem->TexTransform = MathHelper::Identity4x4();
    terrain2Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial2"].get();
    terrain2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

    mRitemLayer[(int)RenderLayer::Scene14].push_back(terrain2Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(terrain2Ritem));

    auto hamburgerRitem = std::make_unique<RenderItem>();
    hamburgerRitem->ObjCBIndex = 774;
    hamburgerRitem->World = MathHelper::Identity4x4();
    hamburgerRitem->TexTransform = MathHelper::Identity4x4();
    hamburgerRitem->Mat = GeometryManager::mMaterials["HamburgerMaterial"].get();
    hamburgerRitem->Geo = GeometryManager::mGeometries["scene15Geo"].get();
    hamburgerRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    hamburgerRitem->IndexCount = hamburgerRitem->Geo->DrawArgs["Hamburger"].IndexCount;
    hamburgerRitem->StartIndexLocation = hamburgerRitem->Geo->DrawArgs["Hamburger"].StartIndexLocation;
    hamburgerRitem->BaseVertexLocation = hamburgerRitem->Geo->DrawArgs["Hamburger"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene15].push_back(hamburgerRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(hamburgerRitem));

    auto marchingCubesRitem = std::make_unique<RenderItem>();
    marchingCubesRitem->ObjCBIndex = 775;
    marchingCubesRitem->World = MathHelper::Identity4x4();
    marchingCubesRitem->TexTransform = MathHelper::Identity4x4();
    marchingCubesRitem->Mat = GeometryManager::mMaterials["TerrainMaterial2"].get();
    marchingCubesRitem->Geo = GeometryManager::mGeometries["MarchingCubes"].get();
    marchingCubesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    mRitemLayer[(int)RenderLayer::Scene17].push_back(marchingCubesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(marchingCubesRitem));


    for (int i = 0; i < GeometryManager::mAllRitems.size(); ++i)
    {
        if (i < 5)
            mRitemLayer[(int)RenderLayer::Opaque].push_back(GeometryManager::mAllRitems[i].get());
        else if (i < 10)
            mRitemLayer[(int)RenderLayer::OpaqueWireframe].push_back(GeometryManager::mAllRitems[i].get());
    }
}

void Engine::BuildBillboardSpritesGeometry()
{
    struct PlanetSpriteVertex
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Size;
    };

    static const int planetCount = 2;
    std::array<PlanetSpriteVertex, 2> vertices;
    for (UINT i = 0; i < 2; ++i)
    {
        float x = 45.f * pow(-1, i);
        float z = 45.f * pow(-1, i);
        float y = 35.f;

        vertices[i].Pos = DirectX::XMFLOAT3(x, y, z);
        vertices[i].Size = DirectX::XMFLOAT2(20.0f, 20.0f);
    }

    std::array<std::uint16_t, 16> indices =
    {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    };

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(PlanetSpriteVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "planetSpritesGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
        mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(PlanetSpriteVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount = (UINT)indices.size();
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;

    geo->DrawArgs["points"] = submesh;

    GeometryManager::mGeometries["planetSpritesGeo"] = std::move(geo);
}

void Engine::InitGBuffer()
{
    gBuffer = GBuffer();

    D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mClientWidth;
	texDesc.Height = mClientHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    
    mDepthStencilBuffer.Reset();

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = mClientWidth;
    depthStencilDesc.Height = mClientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = mDepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    auto tmp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &tmp,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

    auto tmp2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &tmp2,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(mDepthStencilBufferScene3.GetAddressOf())));

    auto tmp3 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(md3dDevice->CreateCommittedResource(
        &tmp3,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(mDepthStencilBufferScene13.GetAddressOf())));


    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    md3dDevice->CreateDepthStencilView(mDepthStencilBufferScene3.Get(), &dsvDesc, DepthStencilViewScene3());
    md3dDevice->CreateDepthStencilView(mDepthStencilBufferScene13.Get(), &dsvDesc, DepthStencilViewScene13());

    gBuffer.gBufferDepth = mDepthStencilBuffer.Get();


	auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	md3dDevice->CreateCommittedResource(
		&heapType,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&gBuffer.gBufferAlbedo));

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferPosition));
	
	md3dDevice->CreateCommittedResource(
		&heapType,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&gBuffer.gBufferNormal));

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferSpecular));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(2, mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferAlbedo.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferPosition.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferNormal.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferSpecular.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, mRtvDescriptorSize);


    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(11, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(gBuffer.gBufferAlbedo.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferPosition.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferNormal.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferSpecular.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    srvDRDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    md3dDevice->CreateShaderResourceView(gBuffer.gBufferDepth.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

void Engine::CreateScene3RTV()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&mRenderTargetBufferScene3));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(6, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(Scene3RenderTargetBuffer(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(19, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(Scene3RenderTargetBuffer(), &srvDRDesc, hDescriptor);
}

void Engine::CreateScene13RTV()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = mBackBufferFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&mRenderTargetBufferScene13));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(8, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(Scene13RenderTargetBuffer(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(58, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = mBackBufferFormat;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(Scene13RenderTargetBuffer(), &srvDRDesc, hDescriptor);
}

void Engine::CreateScene15RTV()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = mBackBufferFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&TAAUtility::m_RTVs[0]));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(9, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(TAAUtility::m_RTVs[0].Get(), nullptr, rtvHandle);

    rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&TAAUtility::m_RTVs[1]));

    md3dDevice->CreateRenderTargetView(TAAUtility::m_RTVs[1].Get(), nullptr, rtvHandle);

    rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&TAAUtility::m_VelocityBuffer));

    md3dDevice->CreateRenderTargetView(TAAUtility::m_VelocityBuffer.Get(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(67, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = mBackBufferFormat;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(TAAUtility::m_RTVs[0].Get(), &srvDRDesc, hDescriptor);
    
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(TAAUtility::m_RTVs[1].Get(), &srvDRDesc, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(TAAUtility::m_VelocityBuffer.Get(), &srvDRDesc, hDescriptor);
}

void Engine::ResizeGBuffer()
{
    FlushCommandQueue();

    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    gBuffer.gBufferAlbedo.Reset();
    gBuffer.gBufferPosition.Reset();
    gBuffer.gBufferNormal.Reset();
    gBuffer.gBufferSpecular.Reset();
    gBuffer.gBufferDepth.Reset();

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // Формат для позиций и нормалей
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferAlbedo));

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferPosition));

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferNormal));

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&gBuffer.gBufferSpecular));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(2, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferAlbedo.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferPosition.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferNormal.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferSpecular.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, mRtvDescriptorSize);

    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();
}

void Engine::InitInstanceBuffer()
{
    int instanceCount = 30 * 30;
    std::vector<InstanceData> instanceData(instanceCount);
    for (UINT i = 0; i < instanceCount; ++i)
    {
        int x = i % 30;
        int y = i / 30;
        int index = 0;
        if (y >= x)
            index = y * y + x;
        else index = (x + 1) * (x + 1) - y - 1;

        DirectX::XMStoreFloat4x4(&instanceData[index].WorldMatrix,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation((float)x * 2, 0, (float)y * 2)));

        instanceData[index].Color = DirectX::XMFLOAT4(1.0f - (float)i/ instanceCount, 1.0f - (float)i / instanceCount, 1.0f - (float)i / instanceCount, 1.0f);
    }
    instancesData = instanceData;

    const UINT instanceBufferSize = instanceCount * sizeof(InstanceData);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    md3dDevice->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&instanceBuffer));

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    md3dDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceUploadBuffer));

    D3D12_SUBRESOURCE_DATA instanceDataSub = 
    {
        .pData = instanceData.data(),
        .RowPitch = instanceBufferSize,
        .SlicePitch = instanceBufferSize
    };

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    mCommandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(mCommandList.Get(),
        instanceBuffer.Get(),
        instanceUploadBuffer.Get(),
        0, 0, 1, &instanceDataSub);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    mCommandList->ResourceBarrier(1, &barrier2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = instanceCount;
    srvDesc.Buffer.StructureByteStride = sizeof(InstanceData);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(16, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(
        instanceBuffer.Get(),
        &srvDesc,
        hDescriptor);
}

void Engine::InitInstanceBufferRMDemo()
{
    int instanceCount = 100;
    std::vector<InstanceDataRMDemo> instanceData(instanceCount);
    for (UINT i = 0; i < instanceCount; ++i)
    {
        int x = i % 10;
        int y = i / 10;
        int index = 0;
        if (y >= x)
            index = y * y + x;
        else index = (x + 1) * (x + 1) - y - 1;

        DirectX::XMStoreFloat4x4(&instanceData[index].WorldMatrix,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation((float)x * 5, 0, (float)y * 5)));

        instanceData[index].Metallic = (float)x / 10 + 0.1f;
        instanceData[index].Roughness = (float)y / 10 + 0.1f;
    }
    instancesDataRMDemo = instanceData;

    const UINT instanceBufferSize = instanceCount * sizeof(InstanceDataRMDemo);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    md3dDevice->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&instanceRMDemoBuffer));

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    md3dDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceRMDemoUploadBuffer));

    D3D12_SUBRESOURCE_DATA instanceDataSub =
    {
        .pData = instanceData.data(),
        .RowPitch = instanceBufferSize,
        .SlicePitch = instanceBufferSize
    };

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceRMDemoBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    mCommandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(mCommandList.Get(),
        instanceRMDemoBuffer.Get(),
        instanceRMDemoUploadBuffer.Get(),
        0, 0, 1, &instanceDataSub);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceRMDemoBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    mCommandList->ResourceBarrier(1, &barrier2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = 100;
    srvDesc.Buffer.StructureByteStride = sizeof(InstanceDataRMDemo);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(53, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(
        instanceRMDemoBuffer.Get(),
        &srvDesc,
        hDescriptor);
}

void Engine::InitInstanceBufferMoreLight()
{
    int instanceCount = 500;
    std::vector<InstanceDataMoreLight> instanceData(instanceCount);
    for (UINT i = 0; i < instanceCount; ++i)
    {
        DirectX::XMStoreFloat4x4(&instanceData[i].WorldMatrix,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(
                float(((i / 100)) * 3), float(((i % 10)) * 2), float(((i % 100) / 10) * 3)
            )));
        instanceData[i].lightID = i + 6;
    }
    instancesDataMoreLight = instanceData;

    const UINT instanceBufferSize = instanceCount * sizeof(InstanceDataMoreLight);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    md3dDevice->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&instanceMoreLightBuffer));

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    md3dDevice->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceMoreLightUploadBuffer));

    D3D12_SUBRESOURCE_DATA instanceDataSub =
    {
        .pData = instanceData.data(),
        .RowPitch = instanceBufferSize,
        .SlicePitch = instanceBufferSize
    };

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceMoreLightBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    mCommandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(mCommandList.Get(),
        instanceMoreLightBuffer.Get(),
        instanceMoreLightUploadBuffer.Get(),
        0, 0, 1, &instanceDataSub);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceMoreLightBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    mCommandList->ResourceBarrier(1, &barrier2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = 500;
    srvDesc.Buffer.StructureByteStride = sizeof(InstanceDataMoreLight);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(56, mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(
        instanceMoreLightBuffer.Get(),
        &srvDesc,
        hDescriptor);
}

void Engine::InitParticleSystem()
{
    mParticleSystem = new ParticleSystem(64, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 1);

    mParticleSystem->InitializeSystem(md3dDevice,
        particleBuffers,
        particleArgsBuffer,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize,
        0);

    mParticleSystem->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);


    mParticleSystem2 = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 2);

    mParticleSystem2->InitializeSystem(md3dDevice,
        particle2Buffers,
        particle2ArgsBuffer,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize, 
        1);

    mParticleSystem2->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);

    mParticleSystemSmoke = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 0.5f, 0.f), 3);

    mParticleSystemSmoke->InitializeSystem(md3dDevice,
        particleSmokeBuffers,
        particleSmokeArgsBuffer,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize,
        2);

    mParticleSystemSmoke->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);

    mParticleSystemRain = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 30.f, 0.f), 4);

    mParticleSystemRain->InitializeSystem(md3dDevice,
        particleRainBuffers,
        particleRainArgsBuffer,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize,
        3);

    mParticleSystemRain->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);
}

void Engine::InitMarchingCubesSystem()
{
    m_MarchingCubes = new MarchingCubes(300, 200, 300, 2.0f);
    m_MarchingCubes->Initialize(md3dDevice.Get(), mCommandList.Get(), GeometryManager::mGeometries);
}

void Engine::CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc, std::string rootSigName)
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

    ThrowIfFailed(md3dDevice->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignatures[rootSigName])));
}

void Engine::DrawSkybox(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            if (i == 3)
                cmdList->DrawIndexedInstanced(ri->IndexCount, instancingLevel * instancingLevel, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
            else if (i == 4 && fpsObjectIsActive)
                cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
            else if (i != 4) cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);    
        }
    }
}

void Engine::DrawRenderItemsScene3(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    if (isUsingInstancingScene3)
    {
        if (!isFrustumCullingScene3)
        {
            auto ri = ritems[0];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 100 * 100, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
        else
        {
            auto ri = ritems[0];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            // Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
            // the heap and set as a root descriptor.
            auto instanceBuffer = CBManager::mCurrFrameResource->InstancingCB->Resource();

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootShaderResourceView(1, instanceBuffer->GetGPUVirtualAddress());
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
    else
    {
        for (size_t i = 0; i < ritems.size(); ++i)
        {
            {
                auto ri = ritems[i];

                auto tmp1 = ri->Geo->VertexBufferView();
                auto tmp2 = ri->Geo->IndexBufferView();
                cmdList->IASetVertexBuffers(0, 1, &tmp1);
                cmdList->IASetIndexBuffer(&tmp2);
                cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

                CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

                D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
                D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

                CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

                cmdList->SetGraphicsRootDescriptorTable(0, tex);
                cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
                cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
                cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

                if (i == 0 && isUsingInstancingScene3)
                    cmdList->DrawIndexedInstanced(ri->IndexCount, 100 * 100, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
                else if (!isUsingInstancingScene3 && i != 0)
                    cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
            }
        }
    }
}

void Engine::DrawRenderItemsScene3LOD(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    {
        auto ri = ritems[LODScene3];

        auto tmp1 = ri->Geo->VertexBufferView();
        auto tmp2 = ri->Geo->IndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

        cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
        cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
    }
}

void Engine::DrawRenderItemsScene4(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene5(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene6(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene6Shadows(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(26, mCbvSrvDescriptorSize);
            cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
            cmdList->SetGraphicsRootDescriptorTable(1, tex);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene7(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (int i = 0; i < Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = Sponza.meshes[i].vertexBufferView;
        auto tmp2 = Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(Sponza.meshes[i].materialIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        shadowMap.Offset(24, mCbvSrvDescriptorSize);
        cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
        cmdList->SetGraphicsRootDescriptorTable(1, tex);
        cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

        cmdList->DrawIndexedInstanced(Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
    }

    if (isObjectsActiveScene7)
    {
        for (size_t i = 0; i < ritems.size(); ++i)
        {
            auto ri = ritems[i];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, mCbvSrvDescriptorSize);
            cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
            cmdList->SetGraphicsRootDescriptorTable(1, tex);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, 0, 0, 0);
        }
    }
}

void Engine::DrawRenderItemsScene7Cascaded(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (int i = 0; i < Sponza.meshes.size(); ++i)
    {
        DirectX::XMFLOAT4X4 worldMat = MathHelper::Identity4x4();
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&worldMat);
        if (IsInCameraView(*mCamera, world, Sponza.meshes[i].boundingBox))
        {
            auto ri = GeometryManager::mAllRitems[747].get();

            auto tmp1 = Sponza.meshes[i].vertexBufferView;
            auto tmp2 = Sponza.meshes[i].indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(Sponza.meshes[i].materialIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, mCbvSrvDescriptorSize);
            cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
            cmdList->SetGraphicsRootDescriptorTable(1, tex);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
        }
    }

    if (isObjectsActiveScene7)
    {
        for (size_t i = 0; i < ritems.size(); ++i)
        {
            auto ri = ritems[i];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, mCbvSrvDescriptorSize);
            cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
            cmdList->SetGraphicsRootDescriptorTable(1, tex);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, 0, 0, 0);
        }
    }
}

void Engine::DrawRenderItemsScene7Shadows(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();


    for (int i = 0; i < Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = Sponza.meshes[i].vertexBufferView;
        auto tmp2 = Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(Sponza.meshes[i].materialIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
    }

    if (isObjectsActiveScene7)
    {
        for (size_t i = 0; i < ritems.size(); ++i)
        {
            auto ri = ritems[i];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, 0, 0, 0);
        }
    }
}

void Engine::DrawRenderItemsScene9(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene9RMDemo(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 100, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene12(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene13(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, GeometryManager::mAllRitems[770]->InstanceCount, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene14(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawTerrainScene14(ID3D12GraphicsCommandList* cmdList)
{
    DirectX::XMMATRIX view = mCamera->GetView();
    DirectX::XMMATRIX proj = mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < m_terrainQuadTree.m_NodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = m_terrainQuadTree.m_NodesToRender[i]->mesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
        }
    }
}

void Engine::DrawDebugTerrainScene14(ID3D12GraphicsCommandList* cmdList)
{
    DirectX::XMMATRIX view = mCamera->GetView();
    DirectX::XMMATRIX proj = mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < m_terrainQuadTree.m_DebugNodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = m_terrainQuadTree.m_DebugNodesToRender[i]->debugMesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
        }
    }
}

void Engine::DrawRenderItemsScene15(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene15TAA(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(TAAObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->TAAObjectsCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawTerrainScene16(ID3D12GraphicsCommandList* cmdList)
{
    DirectX::XMMATRIX view = mCamera->GetView();
    DirectX::XMMATRIX proj = mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < m_terrainQuadTree.m_NodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = m_terrainQuadTree.m_NodesToRender[i]->mesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
        }
    }
}

void Engine::DrawMarchingCubesScene17(ID3D12GraphicsCommandList* cmdList)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto ri = GeometryManager::mAllRitems[775].get();
    auto tmp1 = ri->Geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

    CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

    D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

    cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
    cmdList->SetGraphicsRootDescriptorTable(2, tex);
    cmdList->DrawInstanced(m_MarchingCubes->m_vertices.size(), 1, 0, 0);
}


void Engine::DrawOctreeScene13(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

            cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawDebugGeometryScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();

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

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

            cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawMoreLightGeometryScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->LightObjectCB->Resource();

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

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + 4 * objCBByteSize;
            CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            srvs.Offset(11, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, srvs);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 500, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawDefferedPointSpotScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(LightObjectConstants));

    auto objectCB = CBManager::mCurrFrameResource->LightObjectCB->Resource();

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

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + (ri->ObjCBIndex - 751) * objCBByteSize;
            CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            srvs.Offset(11, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, srvs);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawRenderItemsScene11(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, mCbvSrvDescriptorSize);

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootDescriptorTable(1, instanceTableHandle);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawSponzaScene(ID3D12GraphicsCommandList* cmdList)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (int i = 0; i < Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = Sponza.meshes[i].vertexBufferView;
        auto tmp2 = Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(Sponza.meshes[i].materialIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
    }
}

void Engine::DrawDebugRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    for (size_t i = 0; i < ritems.size(); ++i)
    {
        {
            auto ri = ritems[i];

            auto tmp1 = ri->Geo->VertexBufferView();
            auto tmp2 = ri->Geo->IndexBufferView();
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

            cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

            if (i == 3 && gridIsActive)
                cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
            if (i != 3)
                cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawBillboardRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, ri->StartIndexLocation);
        }
    }
}

void Engine::DrawScreenQuad(ID3D12GraphicsCommandList* cmdList)
{
    auto geo = GeometryManager::mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srvs.Offset(11, mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srvs);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawScreenQuadPostProcessing(ID3D12GraphicsCommandList* cmdList)
{
    auto geo = GeometryManager::mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv.Offset(20, mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srv);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawScreenQuadTAA(ID3D12GraphicsCommandList* cmdList)
{
    auto geo = GeometryManager::mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv1(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv1.Offset(67 + (TAAUtility::m_CurrentRTV + 1) % 2, mCbvSrvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srv2(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv2.Offset(67 + TAAUtility::m_CurrentRTV, mCbvSrvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srv3(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv3.Offset(69, mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srv1);
    cmdList->SetGraphicsRootDescriptorTable(1, srv2);
    cmdList->SetGraphicsRootDescriptorTable(3, srv3);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawScreenQuadAtmosphere(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    cmdList->DrawInstanced(3, 1, 0, 0);
}

void Engine::DrawParticles(ParticleSystem particleSystem, RenderLayer layer)
{
    particleSystem.Render(mCommandList.Get(),
        mRitemLayer[(int)layer],
        mCbvSrvDescriptorSize,
        CBManager::mCurrFrameResource,
        mParticlesSrvUavHeap);
}

void Engine::DrawParticlesGPU(ParticleSystem particleSystem, RenderLayer layer, UINT CB1, UINT SRV1)
{
    UINT id = 0;
    if (layer == RenderLayer::Particles1)
        id = 0;
    else if (layer == RenderLayer::Particles2)
        id = 1;
    else if (layer == RenderLayer::Particles3)
        id = 2;

    particleSystem.RenderGPU(mCommandList.Get(),
        mRitemLayer[(int)layer],
        mCbvSrvDescriptorSize,
        CBManager::mCurrFrameResource,
        mParticlesSrvUavHeap,
        id,
        mRootSignatures, CB1, SRV1, md3dDevice, GeometryManager::mTextures["ParticleSmokeTex"]->Resource);
}

void Engine::DrawSceneToShadowMap()
{
    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorsRect;
    CD3DX12_RESOURCE_BARRIER barrier1;
    CD3DX12_RESOURCE_BARRIER barrier2;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;

    if (!isUsingCascadedShadowsScene7)
    {
        if (shadowSizeIDScene7 == 3)
        {
            viewport = mShadowMap2048->Viewport();
            scissorsRect = mShadowMap2048->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap2048->Dsv();
        }
        else if (shadowSizeIDScene7 == 2)
        {
            viewport = mShadowMap1024->Viewport();
            scissorsRect = mShadowMap1024->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap1024->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap1024->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap1024->Dsv();
        }
        else if (shadowSizeIDScene7 == 1)
        {
            viewport = mShadowMap512->Viewport();
            scissorsRect = mShadowMap512->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap512->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap512->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap512->Dsv();
        }
        else if (shadowSizeIDScene7 == 0)
        {
            viewport = mShadowMap256->Viewport();
            scissorsRect = mShadowMap256->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap256->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap256->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap256->Dsv();
        }
        else 
        {
            viewport = mShadowMap2048->Viewport();
            scissorsRect = mShadowMap2048->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap2048->Dsv();
        }
    }
    else
    {
        viewport = mShadowMap2048->Viewport();
        scissorsRect = mShadowMap2048->ScissorRect();
        barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
        dsv = mShadowMap2048->Dsv();
    }

    mCommandList->RSSetViewports(1, &viewport);
    mCommandList->RSSetScissorRects(1, &scissorsRect);

    mCommandList->ResourceBarrier(1, &barrier1);

    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    mCommandList->OMSetRenderTargets(0, nullptr, false, &dsv);

    mCommandList->SetPipelineState(mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene7Shadows(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene7]);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &barrier2);
}

void Engine::DrawParticlesSceneToShadowMap()
{
    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorsRect;
    CD3DX12_RESOURCE_BARRIER barrier1;
    CD3DX12_RESOURCE_BARRIER barrier2;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;

    {
        viewport = mShadowMapScene6->Viewport();
        scissorsRect = mShadowMapScene6->ScissorRect();
        barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMapScene6->Resource(),
            D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMapScene6->Resource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
        dsv = mShadowMapScene6->Dsv();
    }

    mCommandList->RSSetViewports(1, &viewport);
    mCommandList->RSSetScissorRects(1, &scissorsRect);

    mCommandList->ResourceBarrier(1, &barrier1);

    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    mCommandList->OMSetRenderTargets(0, nullptr, false, &dsv);

    mCommandList->SetPipelineState(mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene6(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene6_3]);

    // Particles
    ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { mParticlesSrvUavHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

    mCommandList->SetGraphicsRootSignature(mRootSignatures["mRootSignatureParticlesRender"].Get());

    auto passCB = CBManager::mCurrFrameResource->ShadowPassCBParticles->Resource();
    mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(10 + currParticle2ReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
    mCommandList->SetGraphicsRootDescriptorTable(1, handle);

    mCommandList->SetPipelineState(mPSOs["shadowParticlesPSO"].Get());

    DrawParticles(*mParticleSystemSmoke, RenderLayer::Particles3);
    
    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &barrier2);
}

void Engine::DrawSceneToShadowMapCascaded(int cascadedMapID)
{
    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorsRect;
    CD3DX12_RESOURCE_BARRIER barrier1;
    CD3DX12_RESOURCE_BARRIER barrier2;
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;

    {
        if (cascadedMapID == 3)
        {
            viewport = mShadowMap2048->Viewport();
            scissorsRect = mShadowMap2048->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap2048->Dsv();
        }
        else if (cascadedMapID == 2)
        {
            viewport = mShadowMap1024->Viewport();
            scissorsRect = mShadowMap1024->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap1024->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap1024->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap1024->Dsv();
        }
        else if (cascadedMapID == 1)
        {
            viewport = mShadowMap512->Viewport();
            scissorsRect = mShadowMap512->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap512->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap512->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap512->Dsv();
        }
        else if (cascadedMapID == 0)
        {
            viewport = mShadowMap256->Viewport();
            scissorsRect = mShadowMap256->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap256->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap256->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap256->Dsv();
        }
        else
        {
            viewport = mShadowMap2048->Viewport();
            scissorsRect = mShadowMap2048->ScissorRect();
            barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mShadowMap2048->Resource(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
            dsv = mShadowMap2048->Dsv();
        }
    }


    mCommandList->RSSetViewports(1, &viewport);
    mCommandList->RSSetScissorRects(1, &scissorsRect);

    mCommandList->ResourceBarrier(1, &barrier1);

    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    mCommandList->OMSetRenderTargets(0, nullptr, false, &dsv);

    mCommandList->SetPipelineState(mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene7Shadows(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene7]);

    // Change back to GENERIC_READ so we can read the texture in a shader.
    mCommandList->ResourceBarrier(1, &barrier2);
}

bool Engine::IsInCameraView(Camera camera, DirectX::XMMATRIX objectPosition, DirectX::BoundingBox itemBox)
{
    DirectX::XMMATRIX view = camera.GetView();
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX invView = XMMatrixInverse(&det, view);

    {
        DirectX::XMMATRIX world = objectPosition;

        auto det2 = XMMatrixDeterminant(world);
        DirectX::XMMATRIX invWorld = DirectX::XMMatrixInverse(&det2, world);

        // View space to the object's local space.
        DirectX::XMMATRIX viewToLocal = DirectX::XMMatrixMultiply(invView, invWorld);

        DirectX::BoundingFrustum mCamFrustumTest;
        DirectX::BoundingFrustum::CreateFromMatrix(mCamFrustumTest, camera.GetProj());

        // Transform the camera frustum from view space to the object's local space.
        DirectX::BoundingFrustum localSpaceFrustum;
        mCamFrustumTest.Transform(localSpaceFrustum, viewToLocal);

        // Perform the box/frustum intersection test in local space.
        if (localSpaceFrustum.Contains(itemBox) != DirectX::DISJOINT)
        {
            return true;
        }
        else return false;
    }
}

std::vector<DirectX::XMFLOAT4> Engine::GetFrustumCornersWorldSpace(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    const auto viewProj = view * proj;
    auto det = XMMatrixDeterminant(viewProj);
    const auto inv = DirectX::XMMatrixInverse(&det, viewProj);

    std::vector<DirectX::XMFLOAT4> frustumCorners;
    frustumCorners.reserve(8);
    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z)
            {
                const DirectX::XMFLOAT4 point(2.0f * x - 1.0f, 2.0f * y - 1.0f, z, 1.0f);
                DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&point);
                pt = DirectX::XMVector4Transform(pt, inv);
                DirectX::XMFLOAT4 resultPoint;
                DirectX::XMStoreFloat4(&resultPoint, pt);
                resultPoint = DirectX::XMFLOAT4(resultPoint.x / resultPoint.w, resultPoint.y / resultPoint.w, resultPoint.z / resultPoint.w, resultPoint.w / resultPoint.w);
                frustumCorners.push_back(resultPoint);
            }
        }
    }

    return frustumCorners;
}

void Engine::CalculateCascadedShadowsCameras()
{
    DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    std::vector<DirectX::XMFLOAT4> corners = GetFrustumCornersWorldSpace(mCameraFrustum0->GetView(), mCameraFrustum0->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());

    float sceneRadius = 50.f;
    // Only the first "main" light casts a shadow.
    DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&mRotatedLightDirections[0]);
    DirectX::XMVECTOR lightPos = DirectX::XMVectorScale(lightDir, -2.0f * sceneRadius);
    DirectX::XMVECTOR targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMFLOAT3 lightPosition;
    DirectX::XMStoreFloat3(&lightPosition, lightPos);
    DirectX::XMFLOAT3 targetPosition;
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    mCameraShadowMap256->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCameraShadowMap256->UpdateViewMatrix();
    float minX = std::numeric_limits<float>::infinity();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::infinity();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::infinity();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, mCameraShadowMap256->GetView());
        DirectX::XMFLOAT4 resultPoint;
        DirectX::XMStoreFloat4(&resultPoint, pt);

        minX = std::min<float>(minX, resultPoint.x);
        maxX = std::max<float>(maxX, resultPoint.x);
        minY = std::min<float>(minY, resultPoint.y);
        maxY = std::max<float>(maxY, resultPoint.y);
        minZ = std::min<float>(minZ, resultPoint.z);
        maxZ = std::max<float>(maxZ, resultPoint.z);
    }

    /*float zMult = 10.0f;
    minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
    maxZ = (maxZ < 0) ? maxZ * zMult : maxZ / zMult;*/

    mCameraShadowMap256->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*mCameraShadowMap256, 0, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(mCameraFrustum1->GetView(), mCameraFrustum1->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    mCameraShadowMap512->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCameraShadowMap512->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, mCameraShadowMap512->GetView());
        DirectX::XMFLOAT4 resultPoint;
        DirectX::XMStoreFloat4(&resultPoint, pt);

        minX = std::min<float>(minX, resultPoint.x);
        maxX = std::max<float>(maxX, resultPoint.x);
        minY = std::min<float>(minY, resultPoint.y);
        maxY = std::max<float>(maxY, resultPoint.y);
        minZ = std::min<float>(minZ, resultPoint.z);
        maxZ = std::max<float>(maxZ, resultPoint.z);
    }

    /*zMult = 10.0f;
    minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
    maxZ = (maxZ < 0) ? maxZ * zMult : maxZ / zMult;*/

    mCameraShadowMap512->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*mCameraShadowMap512, 1, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(mCameraFrustum2->GetView(), mCameraFrustum2->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    mCameraShadowMap1024->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCameraShadowMap1024->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, mCameraShadowMap1024->GetView());
        DirectX::XMFLOAT4 resultPoint;
        DirectX::XMStoreFloat4(&resultPoint, pt);

        minX = std::min<float>(minX, resultPoint.x);
        maxX = std::max<float>(maxX, resultPoint.x);
        minY = std::min<float>(minY, resultPoint.y);
        maxY = std::max<float>(maxY, resultPoint.y);
        minZ = std::min<float>(minZ, resultPoint.z);
        maxZ = std::max<float>(maxZ, resultPoint.z);
    }

    /*zMult = 10.0f;
    minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
    maxZ = (maxZ < 0) ? maxZ * zMult : maxZ / zMult;*/

    mCameraShadowMap1024->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*mCameraShadowMap1024, 2, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(mCameraFrustum3->GetView(), mCameraFrustum3->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    mCameraShadowMap2048->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCameraShadowMap2048->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, mCameraShadowMap2048->GetView());
        DirectX::XMFLOAT4 resultPoint;
        DirectX::XMStoreFloat4(&resultPoint, pt);

        minX = std::min<float>(minX, resultPoint.x);
        maxX = std::max<float>(maxX, resultPoint.x);
        minY = std::min<float>(minY, resultPoint.y);
        maxY = std::max<float>(maxY, resultPoint.y);
        minZ = std::min<float>(minZ, resultPoint.z);
        maxZ = std::max<float>(maxZ, resultPoint.z);
    }

    /*zMult = 10.0f;
    minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
    maxZ = (maxZ < 0) ? maxZ * zMult : maxZ / zMult;*/

    mCameraShadowMap2048->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*mCameraShadowMap2048, 3, center, minX, maxX, minY, maxY, minZ, maxZ);
}
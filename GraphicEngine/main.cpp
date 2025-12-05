#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <vector>
#include <string>

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
#include "ShadowMap.h"
#include "OcTree.h"
#include "QuadTree.h"
#include "TAAUtility.h"
#include "MarchingCubes.h"
#include "SponzaLoader.h"

#include "RootSignatureManager.h"
#include "PSOManager.h"
#include "InputLayoutShaderManager.h"
#include "UIManager.h"
#include "SamplerManager.h"
#include "CBManager.h"
#include "GeometryManager.h"

#include "CommonData.h"

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


struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                      FenceValue;
};



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

    virtual void BuildPostProcessingResources() override;
    virtual void CreateNoiseTexture() override;

    virtual void BuildFrameResources() override;
    virtual void BuildBillboardSpritesGeometry() override;

    virtual void InitGBuffer() override;
    virtual void CreateScene3RTV() override;
    virtual void CreateScene13RTV() override;
    virtual void CreateScene15RTV() override;
    virtual void CreateScene14RTVsSRVs() override;
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

    std::vector<std::vector<StaticMesh>> terrainMeshesLOD;


    // Render items divided by PSO.
    std::vector<RenderItem*> mOpaqueRitems;

    float mTheta = 1.3f * DirectX::XM_PI;
    float mPhi = 0.4f * DirectX::XM_PI;
    float mRadius = 2.5f;

    POINT mLastMousePos;

    float mCameraSpeed = 1.0f;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_UVTexScene14;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_PaintTextureScene14;
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
    CBManager::mCamera->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    CBManager::mCameraFrustum0->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 30.0f, 1000.0f);
    CBManager::mCameraFrustum1->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 20.0f, 30.0f);
    CBManager::mCameraFrustum2->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 10.0f, 20.0f);
    CBManager::mCameraFrustum3->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 10.0f);

    CBManager::mCameraShadowMap256->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    CBManager::mCameraShadowMap512->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    CBManager::mCameraShadowMap1024->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    CBManager::mCameraShadowMap2048->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    CBManager::mCamera2Scene3->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    CBManager::mCamera2Scene13->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    SetCamera2Scene3();
    SetCamera2Scene13();
    DirectX::BoundingFrustum::CreateFromMatrix(mCamFrustum, CBManager::mCamera->GetProj());

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
    CBManager::CBManager::mCurrFrameResource = CBManager::mCurrFrameResource;

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
        CBManager::UpdatePaintClickCB();
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
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), PSOManager::mPSOs["deferredLighting"].Get()));
    }
    else
    {
        ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), PSOManager::mPSOs["shadowForwardPSO"].Get()));
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

    if (!bIsPausedScene15)
        mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);

    if (activeSceneID != 7)
    {
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 6, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 8, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::LightSkyBlue, 0, nullptr);
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
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, DescriptorHeapManager::mRtvDescriptorSize),
    };

    D3D12_CPU_DESCRIPTOR_HANDLE postProcessingRTV = CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, DescriptorHeapManager::mRtvDescriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvsForward[] = 
    {
        CurrentBackBufferView(),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, DescriptorHeapManager::mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, DescriptorHeapManager::mRtvDescriptorSize),
    };

    auto dsv = DepthStencilView();

    ID3D12DescriptorHeap* descriptorHeaps[] = { DescriptorHeapManager::mSrvHeap.Get() };
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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDebug"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["debug"].Get());


            DrawDebugRenderItems(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::OpaqueWireframe]);
        }

        // Draw billboards
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureBillboard"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["billboardSprites"].Get());


            DrawBillboardRenderItems(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::BillboardSprites]);
        }

        // Draw Opaque
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignature"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (isSolid && !isPixelated) mCommandList->SetPipelineState(PSOManager::mPSOs["opaqueSolid"].Get());
            else if (isPixelated) mCommandList->SetPipelineState(PSOManager::mPSOs["opaquePixel"].Get());
            else mCommandList->SetPipelineState(PSOManager::mPSOs["opaqueWireframe"].Get());


            DrawRenderItems(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Opaque]);


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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignature"].Get());

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
            
            mCommandList->SetPipelineState(PSOManager::mPSOs["opaqueSolid"].Get());

            DrawRenderItems(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene2Opaque]);


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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForward"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForwardFrustumCulling"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefaultFrustumCulling"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else if (isUsingInstancingScene3)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForwardFrustumCulling"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefaultFrustumCulling"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForward"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = CBManager::mCurrFrameResource->LODCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, LODCB->GetGPUVirtualAddress());

            auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            mCommandList->ResourceBarrier(1, &backBuffer);

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefault"].Get());

            DrawRenderItemsScene3(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }

        // LOD
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefaultForward"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["forwardDefault"].Get());

            UpdateLODScene3();
            DrawRenderItemsScene3LOD(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene3LOD]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureMoreSamplers"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["moreSamplers"].Get());

            DrawRenderItemsScene4(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene4]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignature"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (isSolidScene5) mCommandList->SetPipelineState(PSOManager::mPSOs["opaqueSolid"].Get());
            else mCommandList->SetPipelineState(PSOManager::mPSOs["opaqueWireframe"].Get());

            DrawRenderItems(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene5]);

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
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleBuffers[1 - currParticleReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->EmitterCB->Resource();

                mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleBuffers[1 - currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleBuffers[currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(PSOManager::mPSOs["computeParticles"].Get());

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
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesRender"].Get());

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(currParticleReadBuffer * 2 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["renderParticles"].Get());

                DrawParticles(*CBManager::mParticleSystem, RenderLayer::Particles1);
            }
        }
        else if (particles2IsActive && activeParticleSystemScene6 == 2)
        {
            // Compute pass
            {
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particle2Buffers[1 - currParticle2ReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter2CB->Resource();

                mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particle2Buffers[1 - currParticle2ReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particle2Buffers[currParticle2ReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(PSOManager::mPSOs["computeParticles"].Get());

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
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesRender"].Get());

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(5 + currParticle2ReadBuffer * 2 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["renderParticles"].Get());

                DrawParticles(*CBManager::mParticleSystem2, RenderLayer::Particles2);
            }
        }
        else if (activeParticleSystemScene6 == 3)
        {
            //// Compute args pass
            //{
            //    ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
            //    mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

            //    CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
            //        particleSmokeArgsBuffer.Get(),
            //        D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
            //        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            //    mCommandList->ResourceBarrier(1, &computeBarrier1);

            //    mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesArgsCompute"].Get());

            //    mCommandList->SetComputeRootUnorderedAccessView(0, particleSmokeArgsBuffer->GetGPUVirtualAddress());

            //    mCommandList->SetPipelineState(PSOManager::mPSOs["computeParticlesArgs"].Get());

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
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleSmokeBuffers[1 - currParticleSmokeReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter3CB->Resource();

                mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleSmokeBuffers[1 - currParticleSmokeReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleSmokeBuffers[currParticleSmokeReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(PSOManager::mPSOs["computeParticles"].Get());

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
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    particleRainBuffers[1 - currParticleRainReadBuffer].Get(),
                    D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                mCommandList->ResourceBarrier(1, &computeBarrier1);

                auto emitterCB = CBManager::mCurrFrameResource->Emitter4CB->Resource();

                mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesCompute"].Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleRainBuffers[1 - currParticleRainReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleRainBuffers[currParticleRainReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(PSOManager::mPSOs["computeParticlesRain"].Get());

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
                ID3D12DescriptorHeap* prticlesDescriptorHeaps[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(prticlesDescriptorHeaps), prticlesDescriptorHeaps);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureShadows"].Get());

                auto passCB = CBManager::mCurrFrameResource->ShadowPassCBParticles->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawParticlesSceneToShadowMap();
            }

            // Draw calls
            // Draw objects
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

                 mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureShadowsParticlesForward"].Get());

                auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
                mCommandList->ResourceBarrier(1, &barrier1);

                mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

                mCommandList->SetPipelineState(PSOManager::mPSOs["shadowForwardParticlesPSO"].Get());

                {
                    passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
                    mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
                    DrawRenderItemsScene6Shadows(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene6_3]);
                }
            }

            // Draw particles Smoke
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesRender"].Get());

                auto passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(10 + currParticleSmokeReadBuffer * 2 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                if (lightingIDScene6 == 2)
                    mCommandList->SetPipelineState(PSOManager::mPSOs["renderParticlesForward"].Get());
                else mCommandList->SetPipelineState(PSOManager::mPSOs["renderParticlesForwardVertexLighting"].Get());

                DrawParticles(*CBManager::mParticleSystemSmoke, RenderLayer::Particles3);
                //DrawParticlesGPU(*CBManager::mParticleSystemSmoke, RenderLayer::Particles3);
            }

            // Draw particles Rain
            {
                ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
                mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesRain"].Get());

                auto passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(15 + currParticleRainReadBuffer * 2 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["particlesRain"].Get());

                DrawParticles(*CBManager::mParticleSystemRain, RenderLayer::RainParticles);

                // Indicate a state transition on the resource usage.
                auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
                mCommandList->ResourceBarrier(1, &barrier2);
            }
        }
    }

    else if (activeSceneID == 7)
    {
        ID3D12DescriptorHeap* descriptorHeapsShadowPass[] = { DescriptorHeapManager::mSponzaSrvHeap.Get() };
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsShadowPass), descriptorHeapsShadowPass);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureShadows"].Get());

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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureShadowsForward"].Get());

        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &barrier1);

        mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

        CD3DX12_GPU_DESCRIPTOR_HANDLE shadowTextures(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        shadowTextures.Offset(28, DescriptorHeapManager::mCbvSrvDescriptorSize);
        mCommandList->SetGraphicsRootDescriptorTable(5, shadowTextures);

        mCommandList->SetPipelineState(PSOManager::mPSOs["shadowForwardPSO"].Get());

        if (isUsingCascadedShadowsScene7)
        {
            passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            DrawRenderItemsScene7Cascaded(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene7]);
        }
        else 
        {
            passCB = CBManager::mCurrFrameResource->PassCBShadows->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            DrawRenderItemsScene7(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene7]);
        }

        // Indicate a state transition on the resource usage.
        auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier2);
    }


    else if (activeSceneID == 8)
    {
        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureRT"].Get());

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

        mCommandList->SetPipelineState(PSOManager::mPSOs["defferedRT"].Get());

        ID3D12DescriptorHeap* descriptorHeapSponza[] = { DescriptorHeapManager::mSponzaSrvHeap.Get() };
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
            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignaturePBR"].Get());

            if (PBRShaderScene9 == 0)
                mCommandList->SetPipelineState(PSOManager::mPSOs["StandartPBRPSO"].Get());
            else if (PBRShaderScene9 == 1)
                mCommandList->SetPipelineState(PSOManager::mPSOs["PBRPSO"].Get());
            else if (PBRShaderScene9 == 2)
                mCommandList->SetPipelineState(PSOManager::mPSOs["IBLPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(48, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(4, tex);

            DrawRenderItemsScene9(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene9]);
        }
        else
        {
            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureRMDemo"].Get());

            mCommandList->SetPipelineState(PSOManager::mPSOs["RMDemoPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(48, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(4, tex);
            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceBuffer(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceBuffer.Offset(53, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(5, instanceBuffer);

            DrawRenderItemsScene9RMDemo(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene9RMDemo]);
        }

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    else if (activeSceneID == 10)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureRT"].Get());

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
                mCommandList->SetPipelineState(PSOManager::mPSOs["defferedRT"].Get());
            else if (isUsingPositionBufferScene10 || selectedRenderTechScene10 == 2)
                mCommandList->SetPipelineState(PSOManager::mPSOs["defferedRT"].Get());
            else mCommandList->SetPipelineState(PSOManager::mPSOs["defferedRTNoPos"].Get());

            ID3D12DescriptorHeap* descriptorHeapSponza[] = { DescriptorHeapManager::mSponzaSrvHeap.Get() };
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeapSponza), descriptorHeapSponza);
            DrawSponzaScene(mCommandList.Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
            DrawRenderItemsScene10(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene10]);

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
                mCommandList->SetPipelineState(PSOManager::mPSOs["forwardRT"].Get());
            else mCommandList->SetPipelineState(PSOManager::mPSOs["forwardRTMoreLight"].Get());

            ID3D12DescriptorHeap* descriptorHeapSponza[] = { DescriptorHeapManager::mSponzaSrvHeap.Get() };
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeapSponza), descriptorHeapSponza);
            DrawSponzaScene(mCommandList.Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
            DrawRenderItemsScene10(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene10]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDefferedPointSpotDirectional"].Get());

            mCommandList->OMSetRenderTargets(1, &rtvs2, false, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["defferedPointSpot"].Get());

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawDefferedPointSpotScene10(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene10DebugGeometry]);

            // More Light
            if (isMoreLightActiveScene10)
            {
                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureMoreLight"].Get());

                mCommandList->OMSetRenderTargets(1, &rtvs2, false, nullptr);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["defferedPointSpotMoreLight"].Get());

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE instanceBuffer(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                instanceBuffer.Offset(56, DescriptorHeapManager::mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(3, instanceBuffer);

                DrawMoreLightGeometryScene10(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene10MoreLight]);
            }

            // Directional & ambient light
            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureLight"].Get());

            mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["defferedDirectional"].Get());
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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTess"].Get());

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
            mCommandList->SetPipelineState(PSOManager::mPSOs["decalsTess"].Get());
        else mCommandList->SetPipelineState(PSOManager::mPSOs["decalsTessWireframe"].Get());

        DrawRenderItemsScene11(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene11]);

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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureHeightMap"].Get());

        if (isWireframeScene12)
            mCommandList->SetPipelineState(PSOManager::mPSOs["heightMapWireframePSO"].Get());
        else mCommandList->SetPipelineState(PSOManager::mPSOs["heightMapSolidPSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
        auto heightCB = CBManager::mCurrFrameResource->HeightMapCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(4, heightCB->GetGPUVirtualAddress());

        DrawRenderItemsScene12(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene12]);

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureObjectsScene13"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["Scene13ObjectsPSO"].Get());

            DrawRenderItemsScene13(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene13]);


            // Draw Octree
            if (isDebugOctreeScene13)
            {
                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureOctreeScene13"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(PSOManager::mPSOs["Scene13OctreePSO"].Get());

                DrawOctreeScene13(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene13Octree]);
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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureObjectsScene13"].Get());

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

            mCommandList->SetPipelineState(PSOManager::mPSOs["Scene13ObjectsPSO"].Get());

            DrawRenderItemsScene13(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene13]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(Scene13RenderTargetBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
    }

    else if (activeSceneID == 14)
    {
        //Paint Compute
        if (bIsPaintingScene14 && CommonData::bIsClicked)
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            CD3DX12_RESOURCE_BARRIER computeBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                m_PaintTextureScene14.Get(),
                D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            mCommandList->ResourceBarrier(1, &computeBarrier1);

            auto paintCB = CBManager::mCurrFrameResource->PaintCB->Resource();

            mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTerrainPaint"].Get());
     
            mCommandList->SetComputeRootConstantBufferView(0, paintCB->GetGPUVirtualAddress());

            CD3DX12_GPU_DESCRIPTOR_HANDLE texUV(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            texUV.Offset(70, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetComputeRootDescriptorTable(1, texUV);
            CD3DX12_GPU_DESCRIPTOR_HANDLE texPaint(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            texPaint.Offset(72, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetComputeRootDescriptorTable(2, texPaint);

            mCommandList->SetPipelineState(PSOManager::mPSOs["terrainPaintPSO"].Get());

            UINT threadGroupCount = 1;
            mCommandList->Dispatch(threadGroupCount, 1, 1);

            CD3DX12_RESOURCE_BARRIER computeBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
                m_PaintTextureScene14.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &computeBarrier2);
        }

        mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 12, DescriptorHeapManager::mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvMain = CurrentBackBufferView();
        CD3DX12_CPU_DESCRIPTOR_HANDLE uvTextureRtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 12, DescriptorHeapManager::mRtvDescriptorSize);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = { rtvMain, uvTextureRtv };

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->OMSetRenderTargets(2, rtvs, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTerrain"].Get());

        if (!isWireframeScene14)
            mCommandList->SetPipelineState(PSOManager::mPSOs["terrainPSO"].Get());
        else mCommandList->SetPipelineState(PSOManager::mPSOs["terrainWireframePSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
        auto terrainCB = CBManager::mCurrFrameResource->TerrainCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(4, terrainCB->GetGPUVirtualAddress());
        if (!isWireframeScene14)
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE texPaint(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            texPaint.Offset(71, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(5, texPaint);
        }

        if (isWireframeScene14)
            DrawDebugTerrainScene14(mCommandList.Get());
        else DrawTerrainScene14(mCommandList.Get());

        // Indicate a state transition on the resource usage.
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        mCommandList->ResourceBarrier(1, &barrier1);
    }

    else if (activeSceneID == 15 && !bIsPausedScene15 || (activeSceneID == 15 && drawScene15))
    {
        if (drawScene15)
        {
            drawScene15 = false;
        }

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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignaturePBR"].Get());

            mCommandList->SetPipelineState(PSOManager::mPSOs["PBRPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawRenderItemsScene15(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene15]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
        else
        {
            {
                CD3DX12_CPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
                CD3DX12_CPU_DESCRIPTOR_HANDLE velocityTextureRTVHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 11, DescriptorHeapManager::mRtvDescriptorSize);
                handle.Offset(9, DescriptorHeapManager::mRtvDescriptorSize);

                mCommandList->ClearRenderTargetView(handle, DirectX::Colors::White, 0, nullptr);
                mCommandList->ClearRenderTargetView(velocityTextureRTVHandle, DirectX::Colors::Black, 0, nullptr);

                D3D12_CPU_DESCRIPTOR_HANDLE rtvsTaa[] =
                {
                    handle,
                    velocityTextureRTVHandle
                };

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->OMSetRenderTargets(2, rtvsTaa, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTAA"].Get());

                mCommandList->SetPipelineState(PSOManager::mPSOs["TAAPSO"].Get());

                passCB = CBManager::mCurrFrameResource->TAAPassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawRenderItemsScene15TAA(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene15]);

                /*mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureSkyBox"].Get());

                mCommandList->OMSetRenderTargets(1, &handle, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["skyboxPSO"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                DrawSkybox(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Sky]);*/
            }

            {
                D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

                mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

                mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTAASecondPass"].Get());

                passCB = CBManager::mCurrFrameResource->PassCB->Resource();
                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

                mCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(PSOManager::mPSOs["TAASecondPassPSO"].Get());

                DrawScreenQuadTAA(mCommandList.Get());

                mCommandList->CopyResource(TAAUtility::m_RTVs[1].Get(), CurrentBackBuffer());
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

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureTerrain"].Get());

            mCommandList->SetPipelineState(PSOManager::mPSOs["terrainPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
            auto terrainCB = CBManager::mCurrFrameResource->TerrainCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(4, terrainCB->GetGPUVirtualAddress());

            DrawTerrainScene16(mCommandList.Get());

            /*mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureSkyBox"].Get());

            mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["skyboxPSO"].Get());

            passCB = CBManager::mCurrFrameResource->PassCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

            DrawSkybox(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Sky]);*/
        }

        // Atmosphere pass
        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

            // Indicate a state transition on the resource usage.
            D3D12_RESOURCE_BARRIER depthBarrier = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &depthBarrier);

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureAtmosphere"].Get());

            passCB = CBManager::mCurrFrameResource->AtmosphereCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());

            CD3DX12_GPU_DESCRIPTOR_HANDLE depthSrv(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            depthSrv.Offset(15, DescriptorHeapManager::mCbvSrvDescriptorSize);
            mCommandList->SetGraphicsRootDescriptorTable(1, depthSrv);

            mCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(PSOManager::mPSOs["AtmospherePSO"].Get());

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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureCubeMarching"].Get());

        mCommandList->SetPipelineState(PSOManager::mPSOs["CubeMarchingPSO"].Get());

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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureLight"].Get());

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (activeSceneID == 10 && isMoreLightActiveScene10)
            mCommandList->SetPipelineState(PSOManager::mPSOs["defferedLightingMoreLight"].Get());
        else if (!(activeSceneID == 10 && !isUsingPositionBufferScene10))
            mCommandList->SetPipelineState(PSOManager::mPSOs["deferredLighting"].Get());
        else mCommandList->SetPipelineState(PSOManager::mPSOs["deferredLightingNoPos"].Get());

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

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureDebugGeometry"].Get());

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        auto backBuffer = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        mCommandList->ResourceBarrier(1, &backBuffer);

        mCommandList->OMSetRenderTargets(1, &rtvs2, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->SetPipelineState(PSOManager::mPSOs["debugGeometry"].Get());

        DrawDebugGeometryScene10(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene10DebugGeometry]);
    }

    // SkyBox
    if (!(activeSceneID == 15) && activeSceneID != 16)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureSkyBox"].Get());

        mCommandList->OMSetRenderTargets(1, &rtv, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetPipelineState(PSOManager::mPSOs["skyboxPSO"].Get());

        passCB = CBManager::mCurrFrameResource->PassCB->Resource();
        mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

        DrawSkybox(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Sky]);

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

            mCommandList->SetComputeRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureComputeNoise"].Get());

            CD3DX12_GPU_DESCRIPTOR_HANDLE uav(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            uav.Offset(21, DescriptorHeapManager::mCbvSrvDescriptorSize);
            auto noiseCB = CBManager::mCurrFrameResource->NoiseCB->Resource();

            mCommandList->SetComputeRootDescriptorTable(0, uav);
            mCommandList->SetComputeRootConstantBufferView(1, noiseCB->GetGPUVirtualAddress());

            mCommandList->SetPipelineState(PSOManager::mPSOs["computeNoise"].Get());

            UINT threadGroupCountX = WINDOW_WIDTH / 8.0f;
            UINT threadGroupCountY = WINDOW_HEIGHT / 8.0f;
            mCommandList->Dispatch(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignaturePostProcessing"].Get());

        auto postProcessingPassCB = CBManager::mCurrFrameResource->PostProcessingCB->Resource();

        mCommandList->SetGraphicsRootConstantBufferView(1, postProcessingPassCB->GetGPUVirtualAddress());

        CD3DX12_GPU_DESCRIPTOR_HANDLE srv(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        srv.Offset(22, DescriptorHeapManager::mCbvSrvDescriptorSize);

        mCommandList->SetGraphicsRootDescriptorTable(2, srv);

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (isActiveNormalScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_Default"].Get());
        else if (isActiveGCScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_GC"].Get());
        else if (isActiveGBScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_GB"].Get());
        else if (isActiveCAScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_CA"].Get());
        else if (isActiveVigScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_Vig"].Get());
        else if (isActiveNoiseScene8)
            mCommandList->SetPipelineState(PSOManager::mPSOs["postProcessing_Noise"].Get());

        DrawScreenQuadPostProcessing(mCommandList.Get());
    }

    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    auto barrierLast1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mCommandList->ResourceBarrier(1, &barrierLast1);

    // Imgui
    UIManager::RenderUI(mCommandList, GeometryManager::mAllRitems, DescriptorHeapManager::mSrvHeap, DescriptorHeapManager::mCbvSrvDescriptorSize, GeometryManager::mMaterials);

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

            CBManager::mCamera->Pitch(dy);
            CBManager::mCamera->RotateY(dx);
            if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
            {
                CBManager::mCameraFrustum0->Pitch(dy);
                CBManager::mCameraFrustum1->Pitch(dy);
                CBManager::mCameraFrustum2->Pitch(dy);
                CBManager::mCameraFrustum3->Pitch(dy);
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
        CBManager::mCamera->Walk(10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            CBManager::mCameraFrustum0->Walk(10.0f * speed);
            CBManager::mCameraFrustum1->Walk(10.0f * speed);
            CBManager::mCameraFrustum2->Walk(10.0f * speed);
            CBManager::mCameraFrustum3->Walk(10.0f * speed);
        }
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        CBManager::mCamera->Walk(-10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            CBManager::mCameraFrustum0->Walk(-10.0f * speed);
            CBManager::mCameraFrustum1->Walk(-10.0f * speed);
            CBManager::mCameraFrustum2->Walk(-10.0f * speed);
            CBManager::mCameraFrustum3->Walk(-10.0f * speed);
        }
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        CBManager::mCamera->Strafe(-10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            CBManager::mCameraFrustum0->Strafe(-10.0f * speed);
            CBManager::mCameraFrustum1->Strafe(-10.0f * speed);
            CBManager::mCameraFrustum2->Strafe(-10.0f * speed);
            CBManager::mCameraFrustum3->Strafe(-10.0f * speed);
        }
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        CBManager::mCamera->Strafe(10.0f * speed);
        if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
        {
            CBManager::mCameraFrustum0->Strafe(10.0f * speed);
            CBManager::mCameraFrustum1->Strafe(10.0f * speed);
            CBManager::mCameraFrustum2->Strafe(10.0f * speed);
            CBManager::mCameraFrustum3->Strafe(10.0f * speed);
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

    CBManager::mCamera->UpdateViewMatrix();
    if (activeSceneID == 7 && isUsingCascadedShadowsScene7)
    {
        CBManager::mCameraFrustum0->UpdateViewMatrix();
        CBManager::mCameraFrustum1->UpdateViewMatrix();
        CBManager::mCameraFrustum2->UpdateViewMatrix();
        CBManager::mCameraFrustum3->UpdateViewMatrix();
    }
}

void Engine::UpdateCamera(const GameTimer& gt)
{

}

void Engine::SetCamera2Scene3()
{
    CBManager::mCamera2Scene3->LookAt(DirectX::XMFLOAT3(-20.0f, 20.0f, -20.0f), DirectX::XMFLOAT3(30.0f, 0.0f, 30.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCamera2Scene3->UpdateViewMatrix();
}

void Engine::SetCamera2Scene13()
{
    CBManager::mCamera2Scene13->LookAt(DirectX::XMFLOAT3(40.0f, 40.0f, 40.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCamera2Scene13->UpdateViewMatrix();
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
    DirectX::XMMATRIX view = CBManager::mCamera->GetView();
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
    DirectX::XMMATRIX view = CBManager::mCamera->GetView();
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
        DirectX::BoundingFrustum::CreateFromMatrix(camFrustum, CBManager::mCamera->GetProj());

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
    DirectX::XMFLOAT3 cameraPosition = CBManager::mCamera->GetPosition3f();
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


    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(7, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(postProcessingBuffer.Get(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(20, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
        DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        21,
        DescriptorHeapManager::mCbvSrvDescriptorSize);

    md3dDevice->CreateUnorderedAccessView(noiseTexture.Get(), nullptr, &uavDesc, handle);

    handle.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(2, DescriptorHeapManager::mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferAlbedo.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferPosition.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

	md3dDevice->CreateRenderTargetView(gBuffer.gBufferNormal.Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferSpecular.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);


    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(11, DescriptorHeapManager::mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(gBuffer.gBufferAlbedo.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferPosition.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferNormal.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    md3dDevice->CreateShaderResourceView(gBuffer.gBufferSpecular.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    srvDRDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    md3dDevice->CreateShaderResourceView(gBuffer.gBufferDepth.Get(), &srvDRDesc, hDescriptor);
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(6, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(Scene3RenderTargetBuffer(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(19, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(8, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(Scene13RenderTargetBuffer(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(58, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(9, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(TAAUtility::m_RTVs[0].Get(), nullptr, rtvHandle);

    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&TAAUtility::m_RTVs[1]));

    md3dDevice->CreateRenderTargetView(TAAUtility::m_RTVs[1].Get(), nullptr, rtvHandle);

    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    D3D12_RESOURCE_DESC texDescVelocity = {};
    texDescVelocity.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDescVelocity.Alignment = 0;
    texDescVelocity.Width = mClientWidth;
    texDescVelocity.Height = mClientHeight;
    texDescVelocity.DepthOrArraySize = 1;
    texDescVelocity.MipLevels = 1;
    texDescVelocity.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDescVelocity.SampleDesc.Count = 1;
    texDescVelocity.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDescVelocity.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDescVelocity,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&TAAUtility::m_VelocityBuffer));

    md3dDevice->CreateRenderTargetView(TAAUtility::m_VelocityBuffer.Get(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(67, DescriptorHeapManager::mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDRDesc = {};
    srvDRDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDRDesc.Format = mBackBufferFormat;
    srvDRDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDRDesc.Texture2D.MostDetailedMip = 0;
    srvDRDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(TAAUtility::m_RTVs[0].Get(), &srvDRDesc, hDescriptor);
    
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(TAAUtility::m_RTVs[1].Get(), &srvDRDesc, hDescriptor);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvVelocityDesc = {};
    srvVelocityDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvVelocityDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvVelocityDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvVelocityDesc.Texture2D.MostDetailedMip = 0;
    srvVelocityDesc.Texture2D.MipLevels = 1;
    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(TAAUtility::m_VelocityBuffer.Get(), &srvVelocityDesc, hDescriptor);
}

void Engine::CreateScene14RTVsSRVs()
{
    D3D12_RESOURCE_DESC texDescUV = {};
    texDescUV.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDescUV.Alignment = 0;
    texDescUV.Width = mClientWidth;
    texDescUV.Height = mClientHeight;
    texDescUV.DepthOrArraySize = 1;
    texDescUV.MipLevels = 1;
    texDescUV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDescUV.SampleDesc.Count = 1;
    texDescUV.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDescUV.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    auto heapType = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDescUV,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&m_UVTexScene14));

    D3D12_RESOURCE_DESC texDescPaint = {};
    texDescPaint.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDescPaint.Alignment = 0;
    texDescPaint.Width = 1024;
    texDescPaint.Height = 1024;
    texDescPaint.DepthOrArraySize = 1;
    texDescPaint.MipLevels = 1;
    texDescPaint.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDescPaint.SampleDesc.Count = 1;
    texDescPaint.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDescPaint.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    md3dDevice->CreateCommittedResource(
        &heapType,
        D3D12_HEAP_FLAG_NONE,
        &texDescPaint,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&m_PaintTextureScene14));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(12, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(m_UVTexScene14.Get(), nullptr, rtvHandle);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(70, DescriptorHeapManager::mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvUVDesc = {};
    srvUVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvUVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvUVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvUVDesc.Texture2D.MostDetailedMip = 0;
    srvUVDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(m_UVTexScene14.Get(), &srvUVDesc, hDescriptor);

    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    md3dDevice->CreateShaderResourceView(m_PaintTextureScene14.Get(), &srvDesc, hDescriptor);

    hDescriptor.Offset(1, DescriptorHeapManager::mCbvSrvDescriptorSize);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;

    md3dDevice->CreateUnorderedAccessView(
        m_PaintTextureScene14.Get(),
        nullptr,  // No counter resource
        &uavDesc,
        hDescriptor
    );
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeapManager::mRtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.Offset(2, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferAlbedo.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferPosition.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferNormal.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

    md3dDevice->CreateRenderTargetView(gBuffer.gBufferSpecular.Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, DescriptorHeapManager::mRtvDescriptorSize);

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

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(53, DescriptorHeapManager::mCbvSrvDescriptorSize);
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(56, DescriptorHeapManager::mCbvSrvDescriptorSize);
    md3dDevice->CreateShaderResourceView(
        instanceMoreLightBuffer.Get(),
        &srvDesc,
        hDescriptor);
}

void Engine::InitParticleSystem()
{
    CBManager::mParticleSystem = new ParticleSystem(64, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 1);

    CBManager::mParticleSystem->InitializeSystem(md3dDevice,
        particleBuffers,
        particleArgsBuffer,
        DescriptorHeapManager::mParticlesSrvUavHeap,
        DescriptorHeapManager::mCbvSrvUavDescriptorSize,
        0);

    CBManager::mParticleSystem->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);


    CBManager::mParticleSystem2 = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 2);

    CBManager::mParticleSystem2->InitializeSystem(md3dDevice,
        particle2Buffers,
        particle2ArgsBuffer,
        DescriptorHeapManager::mParticlesSrvUavHeap,
        DescriptorHeapManager::mCbvSrvUavDescriptorSize, 
        1);

    CBManager::mParticleSystem2->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);

    CBManager::mParticleSystemSmoke = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 0.5f, 0.f), 3);

    CBManager::mParticleSystemSmoke->InitializeSystem(md3dDevice,
        particleSmokeBuffers,
        particleSmokeArgsBuffer,
        DescriptorHeapManager::mParticlesSrvUavHeap,
        DescriptorHeapManager::mCbvSrvUavDescriptorSize,
        2);

    CBManager::mParticleSystemSmoke->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);

    CBManager::mParticleSystemRain = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 30.f, 0.f), 4);

    CBManager::mParticleSystemRain->InitializeSystem(md3dDevice,
        particleRainBuffers,
        particleRainArgsBuffer,
        DescriptorHeapManager::mParticlesSrvUavHeap,
        DescriptorHeapManager::mCbvSrvUavDescriptorSize,
        3);

    CBManager::mParticleSystemRain->BuildSystemVertexBuffers(GeometryManager::mGeometries,
        md3dDevice,
        mCommandList);
}

void Engine::InitMarchingCubesSystem()
{
    m_MarchingCubes = new MarchingCubes(10, 10, 10, 2.0f);
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
        IID_PPV_ARGS(&RootSignatureManager::mRootSignatures[rootSigName])));
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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

                CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

                D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
                D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

                CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
        instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(26, DescriptorHeapManager::mCbvSrvDescriptorSize);
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

    for (int i = 0; i < GeometryManager::Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = GeometryManager::Sponza.meshes[i].vertexBufferView;
        auto tmp2 = GeometryManager::Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(GeometryManager::Sponza.meshes[i].materialIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        shadowMap.Offset(24, DescriptorHeapManager::mCbvSrvDescriptorSize);
        cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
        cmdList->SetGraphicsRootDescriptorTable(1, tex);
        cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

        cmdList->DrawIndexedInstanced(GeometryManager::Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, DescriptorHeapManager::mCbvSrvDescriptorSize);
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

    for (int i = 0; i < GeometryManager::Sponza.meshes.size(); ++i)
    {
        DirectX::XMFLOAT4X4 worldMat = MathHelper::Identity4x4();
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&worldMat);
        if (IsInCameraView(*CBManager::mCamera, world, GeometryManager::Sponza.meshes[i].boundingBox))
        {
            auto ri = GeometryManager::mAllRitems[747].get();

            auto tmp1 = GeometryManager::Sponza.meshes[i].vertexBufferView;
            auto tmp2 = GeometryManager::Sponza.meshes[i].indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(GeometryManager::Sponza.meshes[i].materialIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, DescriptorHeapManager::mCbvSrvDescriptorSize);
            cmdList->SetGraphicsRootDescriptorTable(0, shadowMap);
            cmdList->SetGraphicsRootDescriptorTable(1, tex);
            cmdList->SetGraphicsRootConstantBufferView(2, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(4, matCBAddress);

            cmdList->DrawIndexedInstanced(GeometryManager::Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMap(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            shadowMap.Offset(24, DescriptorHeapManager::mCbvSrvDescriptorSize);
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


    for (int i = 0; i < GeometryManager::Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = GeometryManager::Sponza.meshes[i].vertexBufferView;
        auto tmp2 = GeometryManager::Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(GeometryManager::Sponza.meshes[i].materialIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(GeometryManager::Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
    DirectX::XMMATRIX view = CBManager::mCamera->GetView();
    DirectX::XMMATRIX proj = CBManager::mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    GeometryManager::m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < GeometryManager::m_terrainQuadTree.m_NodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = GeometryManager::m_terrainQuadTree.m_NodesToRender[i]->mesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
    DirectX::XMMATRIX view = CBManager::mCamera->GetView();
    DirectX::XMMATRIX proj = CBManager::mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    GeometryManager::m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < GeometryManager::m_terrainQuadTree.m_DebugNodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = GeometryManager::m_terrainQuadTree.m_DebugNodesToRender[i]->debugMesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
    DirectX::XMMATRIX view = CBManager::mCamera->GetView();
    DirectX::XMMATRIX proj = CBManager::mCamera->GetProj();
    DirectX::XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX inverseViewMatrix = DirectX::XMMatrixInverse(&det, view);
    DirectX::BoundingFrustum localFrustum;
    DirectX::BoundingFrustum::CreateFromMatrix(localFrustum, proj);
    DirectX::BoundingFrustum worldFrustum;
    localFrustum.Transform(worldFrustum, inverseViewMatrix);
    GeometryManager::m_terrainQuadTree.Update(viewProj, worldFrustum);

    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = CBManager::mCurrFrameResource->ObjectCB->Resource();
    auto matCB = CBManager::mCurrFrameResource->MaterialCB->Resource();

    for (size_t i = 0; i < GeometryManager::m_terrainQuadTree.m_NodesToRender.size(); ++i)
    {
        {
            StaticMesh mesh = GeometryManager::m_terrainQuadTree.m_NodesToRender[i]->mesh;
            auto ri = GeometryManager::mAllRitems[773].get();

            auto tmp1 = mesh.vertexBufferView;
            auto tmp2 = mesh.indexBufferView;
            cmdList->IASetVertexBuffers(0, 1, &tmp1);
            cmdList->IASetIndexBuffer(&tmp2);
            cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

    CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
            CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            srvs.Offset(11, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
            CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            srvs.Offset(11, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

            CD3DX12_GPU_DESCRIPTOR_HANDLE instanceTableHandle(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            instanceTableHandle.Offset(16, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

    for (int i = 0; i < GeometryManager::Sponza.meshes.size(); ++i)
    {
        auto ri = GeometryManager::mAllRitems[747].get();

        auto tmp1 = GeometryManager::Sponza.meshes[i].vertexBufferView;
        auto tmp2 = GeometryManager::Sponza.meshes[i].indexBufferView;
        cmdList->IASetVertexBuffers(0, 1, &tmp1);
        cmdList->IASetIndexBuffer(&tmp2);
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSponzaSrvHeap->GetGPUDescriptorHandleForHeapStart());
        tex.Offset(GeometryManager::Sponza.meshes[i].materialIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

        cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(GeometryManager::Sponza.meshes[i].indices.size(), 1, 0, 0, 0);
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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(ri->Mat->DiffuseSrvHeapIndex, DescriptorHeapManager::mCbvSrvDescriptorSize);

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

    CD3DX12_GPU_DESCRIPTOR_HANDLE srvs(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srvs.Offset(11, DescriptorHeapManager::mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srvs);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawScreenQuadPostProcessing(ID3D12GraphicsCommandList* cmdList)
{
    auto geo = GeometryManager::mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv.Offset(20, DescriptorHeapManager::mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srv);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawScreenQuadTAA(ID3D12GraphicsCommandList* cmdList)
{
    auto geo = GeometryManager::mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv1(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv1.Offset(67 + 0, DescriptorHeapManager::mCbvSrvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srv2(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv2.Offset(67 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE srv3(DescriptorHeapManager::mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv3.Offset(69, DescriptorHeapManager::mCbvSrvDescriptorSize);

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
        GeometryManager::mRitemLayer[(int)layer],
        DescriptorHeapManager::mCbvSrvDescriptorSize,
        CBManager::mCurrFrameResource,
        DescriptorHeapManager::mParticlesSrvUavHeap);
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
        GeometryManager::mRitemLayer[(int)layer],
        DescriptorHeapManager::mCbvSrvDescriptorSize,
        CBManager::mCurrFrameResource,
        DescriptorHeapManager::mParticlesSrvUavHeap,
        id,
        RootSignatureManager::mRootSignatures, CB1, SRV1, md3dDevice, GeometryManager::mTextures["ParticleSmokeTex"]->Resource);
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

    mCommandList->SetPipelineState(PSOManager::mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene7Shadows(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene7]);

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

    mCommandList->SetPipelineState(PSOManager::mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene6(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene6_3]);

    // Particles
    ID3D12DescriptorHeap* descriptorHeapsParticleRender[] = { DescriptorHeapManager::mParticlesSrvUavHeap.Get() };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeapsParticleRender), descriptorHeapsParticleRender);

    mCommandList->SetGraphicsRootSignature(RootSignatureManager::mRootSignatures["mRootSignatureParticlesRender"].Get());

    auto passCB = CBManager::mCurrFrameResource->ShadowPassCBParticles->Resource();
    mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(DescriptorHeapManager::mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(10 + currParticle2ReadBuffer * 2 + 1, DescriptorHeapManager::mCbvSrvDescriptorSize);
    mCommandList->SetGraphicsRootDescriptorTable(1, handle);

    mCommandList->SetPipelineState(PSOManager::mPSOs["shadowParticlesPSO"].Get());

    DrawParticles(*CBManager::mParticleSystemSmoke, RenderLayer::Particles3);
    
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

    mCommandList->SetPipelineState(PSOManager::mPSOs["shadowPSO"].Get());

    DrawRenderItemsScene7Shadows(mCommandList.Get(), GeometryManager::mRitemLayer[(int)RenderLayer::Scene7]);

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
    std::vector<DirectX::XMFLOAT4> corners = GetFrustumCornersWorldSpace(CBManager::mCameraFrustum0->GetView(), CBManager::mCameraFrustum0->GetProj());
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

    CBManager::mCameraShadowMap256->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCameraShadowMap256->UpdateViewMatrix();
    float minX = std::numeric_limits<float>::infinity();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::infinity();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::infinity();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, CBManager::mCameraShadowMap256->GetView());
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

    CBManager::mCameraShadowMap256->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*CBManager::mCameraShadowMap256, 0, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(CBManager::mCameraFrustum1->GetView(), CBManager::mCameraFrustum1->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    CBManager::mCameraShadowMap512->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCameraShadowMap512->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, CBManager::mCameraShadowMap512->GetView());
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

    CBManager::mCameraShadowMap512->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*CBManager::mCameraShadowMap512, 1, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(CBManager::mCameraFrustum2->GetView(), CBManager::mCameraFrustum2->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    CBManager::mCameraShadowMap1024->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCameraShadowMap1024->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, CBManager::mCameraShadowMap1024->GetView());
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

    CBManager::mCameraShadowMap1024->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*CBManager::mCameraShadowMap1024, 2, center, minX, maxX, minY, maxY, minZ, maxZ);

    center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    corners = GetFrustumCornersWorldSpace(CBManager::mCameraFrustum3->GetView(), CBManager::mCameraFrustum3->GetProj());
    for (const auto& v : corners)
    {
        center = DirectX::XMFLOAT3(center.x + v.x, center.y + v.y, center.z + v.z);
    }
    center = DirectX::XMFLOAT3(center.x / corners.size(), center.y / corners.size(), center.z / corners.size());
    targetPos = DirectX::XMLoadFloat3(&center);
    DirectX::XMStoreFloat3(&targetPosition, targetPos);

    CBManager::mCameraShadowMap2048->LookAt(lightPosition, targetPosition, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    CBManager::mCameraShadowMap2048->UpdateViewMatrix();
    minX = std::numeric_limits<float>::infinity();
    maxX = std::numeric_limits<float>::lowest();
    minY = std::numeric_limits<float>::infinity();
    maxY = std::numeric_limits<float>::lowest();
    minZ = std::numeric_limits<float>::infinity();
    maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        DirectX::XMVECTOR pt = DirectX::XMLoadFloat4(&v);
        pt = DirectX::XMVector4Transform(pt, CBManager::mCameraShadowMap2048->GetView());
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

    CBManager::mCameraShadowMap2048->SetLensFromCoords(minX, maxX, minY, maxY, minZ, maxZ);
    CBManager::UpdateShadowTransformCascaded(*CBManager::mCameraShadowMap2048, 3, center, minX, maxX, minY, maxY, minZ, maxZ);
}
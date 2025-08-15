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
#include "Model.h"
#include "Instancing.h"
#include "ParticleSystem.h"

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
    Scene10 = 9,
    Scene11 = 10,
    Count
};


struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                      FenceValue;
};

ExampleDescriptorHeapAllocator mSrvHeapAllocator;
ExampleDescriptorHeapAllocator mSrvHeapAllocator2;
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
    void AnimateMaterials(const GameTimer& gt);
    void UpdateInstanceData(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateMainPassCBScene3Camera2(const GameTimer& gt);
    void UpdateParticleEmitterCB(const GameTimer& gt);
    void UpdateParticleEmitter2CB(const GameTimer& gt);
    void UpdatePostProcessingCB(const GameTimer& gt);
    void UpdateNoiseCB(const GameTimer& gt);
    void UpdateSamplersCB(const GameTimer& gt);
    void UpdateLODCB(const GameTimer& gt);

    void ChangeTileObjectTiles();

    virtual void BuildDescriptorHeaps() override;
    virtual void BuildRootSignature() override;
    virtual void BuildShadersAndInputLayout() override;
    virtual void BuildShapeGeometry() override;
    virtual void BuildScene3Geometry() override;
    virtual void BuildScene4Geometry() override;
    virtual void BuildScene5Geometry() override;
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

    virtual void InitGBuffer() override;
    virtual void CreateScene3RTV() override;
    void ResizeGBuffer();

    virtual void InitInstanceBuffer() override;
    virtual void InitParticleSystem() override;
    

    void RenderUI();

    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene3(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene4(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene5(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawRenderItemsScene11(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawDebugRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawBillboardRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawScreenQuad(ID3D12GraphicsCommandList* cmdList);
    void DrawScreenQuadPostProcessing(ID3D12GraphicsCommandList* cmdList);
    void DrawParticles(ParticleSystem particleSystem, RenderLayer layer);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetMoreStaticSamplers();
    std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> GetLODStaticSamplers();

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureDefaultForward = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureDefaultForwardFrustumCulling = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureLight = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureDebug = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureBillboard = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureParticlesCompute = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureParticlesRender = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignaturePostProcessing = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureComputeNoise = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureMoreSamplers = nullptr;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayoutLight;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mBillboardSpriteInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mParticlesInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mPostProcessingInputLayout;

    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    // Render items divided by PSO.
    std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;
    PassConstants mReflectedPassCB;

    ParticleSystem* mParticleSystem;
    ParticleSystem* mParticleSystem2;

    float mTheta = 1.3f * DirectX::XM_PI;
    float mPhi = 0.4f * DirectX::XM_PI;
    float mRadius = 2.5f;

    POINT mLastMousePos;

    Camera mCamera;
    Camera mCamera2Scene3;

    float tilesCount = 1.0f;

    std::string timeScene2 = "";
    std::vector<InstanceData> instancesData;
    std::vector<DirectX::XMMATRIX> instanceDataScene3;
    DirectX::BoundingFrustum mCamFrustum;
};

// Imgui Variables
bool opened = true;

int activeSceneID = 1;

bool isDebug = true;
bool gridIsActive = false;

bool flashlightIsActive = false;

bool isNegative = false;

int tilesCountInt = 1;
int selectedObjectID = 1;
bool isAnimateMaterial = true;
bool isSolid = true;
bool deferredRenderDisplayInfo = false;
bool isParallaxMapping = false;
bool isDeferredRender = false;

bool fpsObjectIsActive = false;

float displacementLevel = 1.0f;

bool isPixelated = false;
int pixelationFactor = 16;

int instancingLevel = 5;

float Obj1Scale = 1.0f;

float Obj1posX = 0.f;
float Obj1posY = 0.f;
float Obj1posZ = 0.f;

float Obj1rotX = 0.f;
float Obj1rotY = 0.f;
float Obj1rotZ = 0.f;


float Obj2posX = 4.f;
float Obj2posY = 0.f;
float Obj2posZ = 0.f;

float Obj2rotX = 0.f;
float Obj2rotY = 0.f;
float Obj2rotZ = 0.f;

float tessFactor = 1.f;

float col1[3] = { 1.0f, 1.0f, 1.0f };
float col2[3] = { 1.0f, 1.0f, 1.0f };
float col3[3] = { 1.0f, 1.0f, 1.0f };

float lightPos1[3] = { 0.0f, 0.0f, 3.0f };
float lightPos2[3] = { 0.0f, 0.0f, 0.0f };
float lightPos3[3] = { 0.0f, 0.0f, -3.0f };

float light1Strength = 0.5f;
float light2Strength = 0.5f;
float light3Strength = 0.5f;

float light1Distance = 1.0f;
float light2Distance = 1.0f;
float light3Distance = 1.0f;


float lightPosSpot1[3] = { 0.0f, 0.0f, 0.0f };
float spotLight1Direction[3] = { 0.0, 0.0f, 0.0f };
float light1SpotStrength = 0.5f;
float light1SpotDistance = 1.0f;
float spotLight1Power = 64.f;
float colSpot1[3] = { 1.0f, 1.0f, 1.0f };

float light2SpotStrength = 0.5f;
float light2SpotDistance = 3.0f;
float spotLight2Power = 64.f;
float colSpot2[3] = { 1.0f, 1.0f, 1.0f };

bool isFrustumCullingScene3 = false;
bool isDisplayingFrustumCullingInfoScene3 = false;
bool isUsingInstancingScene3 = true;
int levelOfDetailsScene3 = 2;

bool isAnimateMaterialScene4 = false;
int tilesCountScene4 = 1;
int filteringModeScene4 = 0;
int addressModeScene4 = 0;

float tessFactorScene5 = 1.f;
bool isSolidScene5 = true;
float displacementLevelScene5 = 1.0f;


int activeParticleSystemScene6 = 1;

bool particles1IsActive = true;
float particleStartColorScene6[3] = { 1.0f, 1.0f, 1.0f };
float particleEndColorScene6[3] = { 0.0f, 0.0f, 0.0f };
float particleStartSizeScene6 = 1.0f;
float particleEndSizeScene6 = 0.5f;

bool particles2IsActive = false;
float particle2StartColorScene6[3] = { 1.0f, 1.0f, 1.0f };
float particle2EndColorScene6[3] = { 0.0f, 0.0f, 0.0f };
float particle2StartSizeScene6 = 1.0f;
float particle2EndSizeScene6 = 0.5f;

int selectedEffectScene8 = 0;
bool isActiveNormalScene8 = true;
bool isActiveGCScene8 = false;
bool isActiveGBScene8 = false;
bool isActiveCAScene8 = false;
bool isActiveVigScene8 = false;
bool isActiveNoiseScene8 = false;
float gammaRatioScene8 = 2.2f;
float textureSizeScene8 = 1000.f;
DirectX::XMFLOAT2 caDistortionScene8(0.05f, 0.05f);
DirectX::XMFLOAT2 caDirectionScene8(1.0f, 0.0f);
bool gbIsHorizontalScene8 = true;
float caIntensityScene8 = 1.0f;
float caPaddingScene8 = 1.0f;
DirectX::XMFLOAT2 vCenterScene8(0.5f, 0.5f);
float vIntensityScene8 = 1.0f;
float vSmoothnessScene8 = 1.0f;
float vRoundnessScene8 = 1.0f;
float nIntensityScene8 = 0.5f;
float nSizeScene8 = 2.f;

int selectedRenderTechScene10 = 0;
bool deferredRenderDisplayInfoScene10 = false;
bool isWireframeScene10 = false;



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
    if (!D3D12Engine::Initialize())
        return false;

    return true;
}

void Engine::OnResize()
{
    //ResizeGBuffer();
    D3D12Engine::OnResize();

    mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

    mCamera2Scene3.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    SetCamera2Scene3();
    DirectX::BoundingFrustum::CreateFromMatrix(mCamFrustum, mCamera.GetProj());
}

void Engine::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    if (isAnimateMaterialScene4)
        AnimateMaterials(gt);

    if (activeSceneID == 3 && isUsingInstancingScene3 && isFrustumCullingScene3)
        UpdateInstanceData(gt);

    UpdateObjectCBs(gt);
    UpdateMaterialCBs(gt);
    UpdateMainPassCB(gt);
    if (activeSceneID == 3)
    {
        UpdateMainPassCBScene3Camera2(gt);
        UpdateLODCB(gt);
    }
    if (activeSceneID == 4)
    {
        UpdateSamplersCB(gt);
    }
    if (activeSceneID == 6)
    {
        if (activeParticleSystemScene6 == 1)
            UpdateParticleEmitterCB(gt);
        else if (activeParticleSystemScene6 == 2)
            UpdateParticleEmitter2CB(gt);
    }
    if (activeSceneID == 8)
    {
        UpdatePostProcessingCB(gt);
        if (isActiveNoiseScene8)
        {
            UpdateNoiseCB(gt);
        }
    }
    timeScene2 = ParseTime(gt);
}

void Engine::Draw(const GameTimer& gt)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(mDirectCmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSOs["deferredLighting"].Get()));

    D3D12_VIEWPORT viewports[] = { mScreenViewport, mScreenViewport2, mScreenViewport3, mScreenViewport4, mScreenViewportFull };
    D3D12_RECT rects[] = { mScissorRect, mScissorRect2, mScissorRect3, mScissorRect4, mScissorRectFull };

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 6, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mRtvDescriptorSize), DirectX::Colors::Black, 0, nullptr);

    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilViewScene3(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize),
    };

    D3D12_CPU_DESCRIPTOR_HANDLE postProcessingRTV = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 7, mRtvDescriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvsForward[] = {
        CurrentBackBufferView(),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize),
    };

    auto dsv = DepthStencilView();

    ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap.Get() };
    auto passCB = mCurrFrameResource->PassCB->Resource();

    if (activeSceneID == 1)
    {
        // Draw Wireframe (debug)
        if (isDebug)
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatureDebug.Get());

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
            auto depth = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth };
            mCommandList->ResourceBarrier(6, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            mCommandList->SetPipelineState(mPSOs["debug"].Get());


            DrawDebugRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::OpaqueWireframe]);
        }

        // Draw billboards
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatureBillboard.Get());

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

            mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->RSSetViewports(1, &viewports[4]);
            mCommandList->RSSetScissorRects(1, &rects[4]);

            if (isSolid && !isPixelated) mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());
            else if (isPixelated) mCommandList->SetPipelineState(mPSOs["opaquePixel"].Get());
            else mCommandList->SetPipelineState(mPSOs["opaqueWireframe"].Get());


            DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);


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

    else if (activeSceneID == 2)
    {
        // Draw calls
        // Draw Opaque
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

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
            auto passCBCamera2 = mCurrFrameResource->PassCBScene3Camera2->Resource();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatureDefaultForward.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCBCamera2->GetGPUVirtualAddress());
            auto LODCB = mCurrFrameResource->LODCB->Resource();
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
            auto passCBCamera2 = mCurrFrameResource->PassCBScene3Camera2->Resource();

            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatureDefaultForwardFrustumCulling.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCBCamera2->GetGPUVirtualAddress());
            auto LODCB = mCurrFrameResource->LODCB->Resource();
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

            mCommandList->SetGraphicsRootSignature(mRootSignatureDefaultForward.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = mCurrFrameResource->LODCB->Resource();
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

            mCommandList->SetGraphicsRootSignature(mRootSignatureDefaultForwardFrustumCulling.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = mCurrFrameResource->LODCB->Resource();
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

            mCommandList->SetGraphicsRootSignature(mRootSignatureDefaultForward.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto LODCB = mCurrFrameResource->LODCB->Resource();
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
    }
    else if (activeSceneID == 4)
    {
        // Draw calls
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignatureMoreSamplers.Get());

            mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());
            auto samplersCB = mCurrFrameResource->SamplersCB->Resource();
            mCommandList->SetGraphicsRootConstantBufferView(5, samplersCB->GetGPUVirtualAddress());

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

            mCommandList->SetPipelineState(mPSOs["moreSamplers"].Get());

            DrawRenderItemsScene4(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene4]);


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
    else if (activeSceneID == 5)
    {
        //Draw calls
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

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

                auto emitterCB = mCurrFrameResource->EmitterCB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatureParticlesCompute.Get());

                mCommandList->SetComputeRootUnorderedAccessView(0, particleBuffers[1 - currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootShaderResourceView(1, particleBuffers[currParticleReadBuffer]->GetGPUVirtualAddress());
                mCommandList->SetComputeRootConstantBufferView(2, emitterCB->GetGPUVirtualAddress());

                mCommandList->SetPipelineState(mPSOs["computeParticles"].Get());

                UINT threadGroupCount = 1;
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

                mCommandList->SetGraphicsRootSignature(mRootSignatureParticlesRender.Get());

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

                auto emitterCB = mCurrFrameResource->Emitter2CB->Resource();

                mCommandList->SetComputeRootSignature(mRootSignatureParticlesCompute.Get());

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

                mCommandList->SetGraphicsRootSignature(mRootSignatureParticlesRender.Get());

                mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle(mParticlesSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
                handle.Offset(4 + currParticle2ReadBuffer * 2 + 1, mCbvSrvDescriptorSize);
                mCommandList->SetGraphicsRootDescriptorTable(1, handle);

                mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

                mCommandList->RSSetViewports(1, &viewports[4]);
                mCommandList->RSSetScissorRects(1, &rects[4]);

                mCommandList->SetPipelineState(mPSOs["renderParticles"].Get());

                DrawParticles(*mParticleSystem2, RenderLayer::Particles2);
            }
        }
    }
    else if (activeSceneID == 8)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

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

        DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

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

    else if (activeSceneID == 10)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

        mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

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
            auto depth = CD3DX12_RESOURCE_BARRIER::Transition(gBuffer.gBufferDepth.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth };
            mCommandList->ResourceBarrier(6, barriers);

            mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

            mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());

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

            mCommandList->SetPipelineState(mPSOs["forwardRT"].Get());

            DrawRenderItemsScene10(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Scene10]);

            // Indicate a state transition on the resource usage.
            auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            mCommandList->ResourceBarrier(1, &barrier1);
        }
    }

    else if (activeSceneID == 11)
    {
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

        mCommandList->SetGraphicsRootConstantBufferView(3, passCB->GetGPUVirtualAddress());

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

        if (!isWireframeScene10)
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
    

    // Draw screen quad
    if (activeSceneID <= 2 || activeSceneID >= 4 && !(activeSceneID == 10 && selectedRenderTechScene10 == 0))
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

        mCommandList->SetGraphicsRootSignature(mRootSignatureLight.Get());

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetPipelineState(mPSOs["deferredLighting"].Get());

        DrawScreenQuad(mCommandList.Get());

        if (activeSceneID == 8)
        {
            auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(postProcessingBuffer.Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            mCommandList->ResourceBarrier(1, &barrier2);
        }
    }

    if (activeSceneID == 8)
    {
        // Noise compute
        if (isActiveNoiseScene8)
        {
            mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

            mCommandList->SetComputeRootSignature(mRootSignatureComputeNoise.Get());

            CD3DX12_GPU_DESCRIPTOR_HANDLE uav(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            uav.Offset(21, mCbvSrvDescriptorSize);
            auto noiseCB = mCurrFrameResource->NoiseCB->Resource();

            mCommandList->SetComputeRootDescriptorTable(0, uav);
            mCommandList->SetComputeRootConstantBufferView(1, noiseCB->GetGPUVirtualAddress());

            mCommandList->SetPipelineState(mPSOs["computeNoise"].Get());

            UINT threadGroupCountX = WINDOW_WIDTH / 8.0f;
            UINT threadGroupCountY = WINDOW_HEIGHT / 8.0f;
            mCommandList->Dispatch(WINDOW_WIDTH, WINDOW_HEIGHT, 1);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignaturePostProcessing.Get());

        auto postProcessingPassCB = mCurrFrameResource->PostProcessingCB->Resource();

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


    // Imgui
    RenderUI();

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

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

            mCamera.Pitch(dy);
            mCamera.RotateY(dx);
        }

        mLastMousePos.x = x;
        mLastMousePos.y = y;
    }
}

void Engine::OnKeyboardInput(const GameTimer& gt)
{
    const float dt = gt.DeltaTime();

    if (GetAsyncKeyState('W') & 0x8000)
        mCamera.Walk(10.0f * dt);

    if (GetAsyncKeyState('S') & 0x8000)
        mCamera.Walk(-10.0f * dt);

    if (GetAsyncKeyState('A') & 0x8000)
        mCamera.Strafe(-10.0f * dt);

    if (GetAsyncKeyState('D') & 0x8000)
        mCamera.Strafe(10.0f * dt);

    mCamera.UpdateViewMatrix();
}

void Engine::UpdateCamera(const GameTimer& gt)
{

}

void Engine::SetCamera2Scene3()
{
    mCamera2Scene3.LookAt(DirectX::XMFLOAT3(-20.0f, 20.0f, -20.0f), DirectX::XMFLOAT3(30.0f, 0.0f, 30.0f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    mCamera2Scene3.UpdateViewMatrix();
}

void Engine::AnimateMaterials(const GameTimer& gt)
{
    auto animateMat = mMaterials["metalAnimate"].get();

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
    DirectX::XMMATRIX view = mCamera.GetView();
    auto det = XMMatrixDeterminant(view);
    DirectX::XMMATRIX invView = XMMatrixInverse(&det, view);

    auto currInstanceBuffer = mCurrFrameResource->InstancingCB.get();
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
            if (localSpaceFrustum.Contains(mAllRitems[13]->Bounds) != DirectX::DISJOINT && isFrustumCullingScene3)
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

        mAllRitems[13]->InstanceCount = visibleInstanceCount;
    }
}

void Engine::UpdateObjectCBs(const GameTimer& gt)
{
    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    for (int i = 0; i < mAllRitems.size(); ++i)
    {
        // Only update the cbuffer data if the constants have changed.  
        // This needs to be tracked per frame resource.
        if (mAllRitems[i]->NumFramesDirty > 0)
        {
            DirectX::XMMATRIX world = XMLoadFloat4x4(&mAllRitems[i]->World);
            DirectX::XMFLOAT4X4 worldM = MathHelper::Identity4x4();
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
            else if (i == 2)
                world = XMLoadFloat4x4(&worldM) * DirectX::XMMatrixTranslation(10.0f, 0.0f, 0.0f);
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
                * DirectX::XMMatrixRotationAxis(DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f }, DirectX::XM_PI/2)
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
                mAllRitems[i]->Mat->TilesCount = tilesCountScene4;
                mAllRitems[i]->Mat->NumFramesDirty = gNumFrameResources;
            }

            DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&mAllRitems[i]->TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
            if (i <= 4 || i == 745)
                objConstants.isTesselationNeeded = 1.0f;
            else objConstants.isTesselationNeeded = 0.0f;
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

            currObjectCB->CopyData(mAllRitems[i]->ObjCBIndex, objConstants);

            // Next FrameResource need to be updated too.
            mAllRitems[i]->NumFramesDirty--;
        }
    }
}

void Engine::UpdateMaterialCBs(const GameTimer& gt)
{
    auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
    for (auto& e : mMaterials)
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
            XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

            currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

            // Next FrameResource need to be updated too.
            mat->NumFramesDirty--;
        }
    }
}

void Engine::UpdateMainPassCB(const GameTimer& gt)
{
    DirectX::XMMATRIX view = mCamera.GetView();
    DirectX::XMMATRIX proj = mCamera.GetProj();

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
    mMainPassCB.EyePosW = mCamera.GetPosition3f();
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
        mMainPassCB.Lights[7].Direction = mCamera.GetLook3f();
        mMainPassCB.Lights[7].Position = mCamera.GetPosition3f();
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

void Engine::UpdateMainPassCBScene3Camera2(const GameTimer& gt)
{
    DirectX::XMMATRIX view = mCamera2Scene3.GetView();
    DirectX::XMMATRIX proj = mCamera2Scene3.GetProj();

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
    mMainPassCB.EyePosW = mCamera2Scene3.GetPosition3f();
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

void Engine::UpdateParticleEmitterCB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;
    
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

void Engine::UpdateParticleEmitter2CB(const GameTimer& gt)
{
    EmitterConstants emitterConstPass;

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

void Engine::UpdatePostProcessingCB(const GameTimer& gt)
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

void Engine::UpdateNoiseCB(const GameTimer& gt)
{
    NoiseComputeConstants noiseConst;

    noiseConst.TotalTime = gt.TotalTime();

    auto currPassCB = mCurrFrameResource->NoiseCB.get();
    currPassCB->CopyData(0, noiseConst);
}

void Engine::UpdateSamplersCB(const GameTimer& gt)
{
    MoreSamplersConstants samplersConst;

    samplersConst.Flitering = filteringModeScene4;
    samplersConst.AddressMode = addressModeScene4;

    auto currPassCB = mCurrFrameResource->SamplersCB.get();
    currPassCB->CopyData(0, samplersConst);
}

void Engine::UpdateLODCB(const GameTimer& gt)
{
    LODConstants LODConst;

    LODConst.LevelOfDetail = levelOfDetailsScene3;

    auto currPassCB = mCurrFrameResource->LODCB.get();
    currPassCB->CopyData(0, LODConst);
}

void Engine::ChangeTileObjectTiles()
{
    if ((int)tilesCount != tilesCountInt)
    {
        tilesCount = tilesCountInt;
        for (auto& e : mAllRitems)
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

    mTextures[metalAnimateTex->Name] = std::move(metalAnimateTex);

    auto metalAnimateNorm = std::make_unique<Texture>();
    metalAnimateNorm->Name = "MetalNorm";
    metalAnimateNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalNorm.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateNorm->Filename.c_str(),
        metalAnimateNorm->Resource, metalAnimateNorm->UploadHeap);

    mTextures[metalAnimateNorm->Name] = std::move(metalAnimateNorm);

    auto metalAnimateDisplacement = std::make_unique<Texture>();
    metalAnimateDisplacement->Name = "MetalDisplacement";
    metalAnimateDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalDisplacement.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateDisplacement->Filename.c_str(),
        metalAnimateDisplacement->Resource, metalAnimateDisplacement->UploadHeap);

    mTextures[metalAnimateDisplacement->Name] = std::move(metalAnimateDisplacement);

    auto metalRoughness = std::make_unique<Texture>();
    metalRoughness->Name = "MetalRoughness";
    metalRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalRoughness->Filename.c_str(),
        metalRoughness->Resource, metalRoughness->UploadHeap), true);

    mTextures[metalRoughness->Name] = std::move(metalRoughness);

    auto metalAO = std::make_unique<Texture>();
    metalAO->Name = "MetalAO";
    metalAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAO->Filename.c_str(),
        metalAO->Resource, metalAO->UploadHeap), true);

    mTextures[metalAO->Name] = std::move(metalAO);

    auto stoneTex = std::make_unique<Texture>();
    stoneTex->Name = "StoneTex";
    stoneTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneTex->Filename.c_str(),
        stoneTex->Resource, stoneTex->UploadHeap), false);
    mTextures[stoneTex->Name] = std::move(stoneTex);

    auto stoneNorm = std::make_unique<Texture>();
    stoneNorm->Name = "StoneNorm";
    stoneNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneNorm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneNorm->Filename.c_str(),
        stoneNorm->Resource, stoneNorm->UploadHeap), false);
    mTextures[stoneNorm->Name] = std::move(stoneNorm);

    auto stoneDisplacement = std::make_unique<Texture>();
    stoneDisplacement->Name = "StoneDisplacement";
    stoneDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneDisplacement.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneDisplacement->Filename.c_str(),
        stoneDisplacement->Resource, stoneDisplacement->UploadHeap), true);
    mTextures[stoneDisplacement->Name] = std::move(stoneDisplacement);

    auto stoneRoughness = std::make_unique<Texture>();
    stoneRoughness->Name = "StoneRoughness";
    stoneRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneRoughness->Filename.c_str(),
        stoneRoughness->Resource, stoneRoughness->UploadHeap), true);
    mTextures[stoneRoughness->Name] = std::move(stoneRoughness);

    auto stoneAO = std::make_unique<Texture>();
    stoneAO->Name = "StoneAO";
    stoneAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneAO->Filename.c_str(),
        stoneAO->Resource, stoneAO->UploadHeap), true);
    mTextures[stoneAO->Name] = std::move(stoneAO);

    auto planetTex = std::make_unique<Texture>();
    planetTex->Name = "PlanetTex";
    planetTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PlanetTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), planetTex->Filename.c_str(),
        planetTex->Resource, planetTex->UploadHeap), true);
    mTextures[planetTex->Name] = std::move(planetTex);

    auto particleTex = std::make_unique<Texture>();
    particleTex->Name = "ParticleTex";
    particleTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\ParticleTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particleTex->Filename.c_str(),
        particleTex->Resource, particleTex->UploadHeap), true);
    mTextures[particleTex->Name] = std::move(particleTex);

    auto skyboxTex = std::make_unique<Texture>();
    skyboxTex->Name = "SkyboxTex";
    skyboxTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), skyboxTex->Filename.c_str(),
        skyboxTex->Resource, skyboxTex->UploadHeap), true);
    mTextures[skyboxTex->Name] = std::move(skyboxTex);

    auto particle2Tex = std::make_unique<Texture>();
    particle2Tex->Name = "Particle2Tex";
    particle2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Particle2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particle2Tex->Filename.c_str(),
        particle2Tex->Resource, particle2Tex->UploadHeap), true);
    mTextures[particle2Tex->Name] = std::move(particle2Tex);
}

void Engine::BuildDescriptorHeaps()
{
    //
    // Create the SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 23;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

    mSrvHeapAllocator.Create(md3dDevice.Get(), mSrvHeap.Get());

    //
    // Create particles SRV/UAV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvParticlesHeapDesc = {};
    srvParticlesHeapDesc.NumDescriptors = 10;
    srvParticlesHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvParticlesHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvParticlesHeapDesc, IID_PPV_ARGS(&mParticlesSrvUavHeap)));

    mSrvHeapAllocator2.Create(md3dDevice.Get(), mParticlesSrvUavHeap.Get());
}

void Engine::UploadTextures()
{
    //
    // Fill out the heap with actual descriptors.
    //
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto MetalTex = mTextures["MetalTex"]->Resource;
    auto MetalNorm = mTextures["MetalNorm"]->Resource;
    auto MetalDisplacement = mTextures["MetalDisplacement"]->Resource;
    auto MetalRoughness = mTextures["MetalRoughness"]->Resource;
    auto MetalAO = mTextures["MetalAO"]->Resource;


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

    auto StoneTex = mTextures["StoneTex"]->Resource;
    auto StoneNorm = mTextures["StoneNorm"]->Resource;
    auto StoneDisplacement = mTextures["StoneDisplacement"]->Resource;
    auto StoneRoughness = mTextures["StoneRoughness"]->Resource;
    auto StoneAO = mTextures["StoneAO"]->Resource;


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

    auto PlanetTex = mTextures["PlanetTex"]->Resource;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc1 = {};
    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = PlanetTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(PlanetTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto SkyboxTex = mTextures["SkyboxTex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = SkyboxTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(SkyboxTex.Get(), &srvDesc1, hDescriptor);

    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto ParticleTex = mTextures["ParticleTex"]->Resource;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptorParticles(mParticlesSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptorParticles.Offset(8, mCbvSrvDescriptorSize);

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(ParticleTex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);

    auto Particle2Tex = mTextures["Particle2Tex"]->Resource;

    srvDesc1.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc1.Format = ParticleTex->GetDesc().Format;
    srvDesc1.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc1.Texture2D.MostDetailedMip = 0;
    srvDesc1.Texture2D.MipLevels = -1;
    md3dDevice->CreateShaderResourceView(Particle2Tex.Get(), &srvDesc1, hDescriptorParticles);

    hDescriptorParticles.Offset(1, mCbvSrvDescriptorSize);
}


void Engine::BuildRootSignature()
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

    auto staticSamplers = GetStaticSamplers();
    auto moreSamplers = GetMoreStaticSamplers();
    auto lodSamplers = GetLODStaticSamplers();

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

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDesc2, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureLight.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescMoreSamplers, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureMoreSamplers.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescDefaultForward, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureDefaultForward.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescDefaultForwardFrustumCulling, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureDefaultForwardFrustumCulling.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescDebug, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureDebug.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescBillboard, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureBillboard.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescParticlesRender, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureParticlesRender.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescParticlesCompute, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureParticlesCompute.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescPostProcessing, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignaturePostProcessing.GetAddressOf())));

    errorBlob = nullptr;
    serializedRootSig = nullptr;
    hr = D3D12SerializeRootSignature(&rootSigDescComputeNoise, D3D_ROOT_SIGNATURE_VERSION_1,
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
        IID_PPV_ARGS(mRootSignatureComputeNoise.GetAddressOf())));
}


void Engine::BuildShadersAndInputLayout()
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

    mShaders["debugVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "VS", "vs_5_0");
    mShaders["debugPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DebugLayer.hlsl", nullptr, "PS", "ps_5_0");

    mShaders["billboardSpriteVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["billboardSpriteGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["billboardSpritePS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Billboard.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["particlesGS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "GS", "gs_5_1");
    mShaders["particlesPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Particles.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["particlesCS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\ComputeParticles.hlsl", nullptr, "CS_UpdateParticles", "cs_5_1");

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
    mShaders["forwardRT_HS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["forwardRT_DS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["forwardRT_PS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Forward.hlsl", nullptr, "PS", "ps_5_1");

    mShaders["decalsTessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "VS", "vs_5_1");
    mShaders["decalsTessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "HS", "hs_5_1");
    mShaders["decalsTessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "DS", "ds_5_1");
    mShaders["decalsTessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DecalsTessellation.hlsl", nullptr, "PS", "ps_5_1");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    mInputLayoutLight =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mBillboardSpriteInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    mParticlesInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    mPostProcessingInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void Engine::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box2 = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box3 = geoGen.CreateBoxTiling(5.f, 5.f, 5.f, 1, 1);
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

    mGeometries[screenQuad->Name] = std::move(screenQuad);

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

    mGeometries[geo->Name] = std::move(geo);
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

    mGeometries[geo->Name] = std::move(geo);

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

    mGeometries[geo->Name] = std::move(geo);
}

void Engine::BuildScene5Geometry()
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData tesselationBox = geoGen.CreateBox(5.0f, 5.0f, 5.0f, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry tesselationBoxSubmesh;
    tesselationBoxSubmesh.IndexCount = (UINT)tesselationBox.Indices32.size();
    tesselationBoxSubmesh.StartIndexLocation = totalIndexCount;
    tesselationBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += tesselationBoxSubmesh.IndexCount;
    totalVertexCount += tesselationBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < tesselationBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = tesselationBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = tesselationBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = tesselationBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = tesselationBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += tesselationBox.Vertices.size();

    indices.insert(indices.end(), std::begin(tesselationBox.GetIndices16()), std::end(tesselationBox.GetIndices16()));

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

    geo->DrawArgs["tesselationBox"] = tesselationBoxSubmesh;

    mGeometries[geo->Name] = std::move(geo);
}


void Engine::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultForwardPsoDesc;

    //
    // PSO for default forward rendering
    //
    ZeroMemory(&defaultForwardPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    defaultForwardPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    defaultForwardPsoDesc.pRootSignature = mRootSignatureDefaultForward.Get();
    defaultForwardPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardVS"]->GetBufferPointer()),
        mShaders["forwardVS"]->GetBufferSize()
    };
    defaultForwardPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardPS"]->GetBufferPointer()),
        mShaders["forwardPS"]->GetBufferSize()
    };
    defaultForwardPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    defaultForwardPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    defaultForwardPsoDesc.SampleMask = UINT_MAX;
    defaultForwardPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    defaultForwardPsoDesc.NumRenderTargets = 1;
    defaultForwardPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    defaultForwardPsoDesc.SampleDesc.Count = 1;
    defaultForwardPsoDesc.SampleDesc.Quality = 0;
    defaultForwardPsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defaultForwardPsoDesc, IID_PPV_ARGS(&mPSOs["forwardDefault"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC defaultForwardFrustumCullingPsoDesc;
    //
    // PSO for default forward rendering with frustum culling
    //
    defaultForwardFrustumCullingPsoDesc = defaultForwardPsoDesc;
    defaultForwardFrustumCullingPsoDesc.pRootSignature = mRootSignatureDefaultForwardFrustumCulling.Get();
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&defaultForwardFrustumCullingPsoDesc, IID_PPV_ARGS(&mPSOs["forwardDefaultFrustumCulling"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaquePsoDesc.pRootSignature = mRootSignature.Get();
    opaquePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessVS"]->GetBufferPointer()),
        mShaders["tessVS"]->GetBufferSize()
    };
    opaquePsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessHS"]->GetBufferPointer()),
        mShaders["tessHS"]->GetBufferSize()
    };
    opaquePsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessDS"]->GetBufferPointer()),
        mShaders["tessDS"]->GetBufferSize()
    };
    opaquePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessPS"]->GetBufferPointer()),
        mShaders["tessPS"]->GetBufferSize()
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
    opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaqueSolid"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC moreSamplersPsoDesc = opaquePsoDesc;
    moreSamplersPsoDesc.pRootSignature = mRootSignatureMoreSamplers.Get();
    moreSamplersPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["moreSamplersVS"]->GetBufferPointer()),
        mShaders["moreSamplersVS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(mShaders["moreSamplersHS"]->GetBufferPointer()),
        mShaders["moreSamplersHS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(mShaders["moreSamplersDS"]->GetBufferPointer()),
        mShaders["moreSamplersDS"]->GetBufferSize()
    };
    moreSamplersPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["moreSamplersPS"]->GetBufferPointer()),
        mShaders["moreSamplersPS"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&moreSamplersPsoDesc, IID_PPV_ARGS(&mPSOs["moreSamplers"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc;

    //
    // PSO for opaque objects.
    //
    ZeroMemory(&opaqueWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaqueWireframePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    opaqueWireframePsoDesc.pRootSignature = mRootSignature.Get();
    opaqueWireframePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessVS"]->GetBufferPointer()),
        mShaders["tessVS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessHS"]->GetBufferPointer()),
        mShaders["tessHS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessDS"]->GetBufferPointer()),
        mShaders["tessDS"]->GetBufferSize()
    };
    opaqueWireframePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["tessPS"]->GetBufferPointer()),
        mShaders["tessPS"]->GetBufferSize()
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
    opaqueWireframePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaqueWireframe"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDescPixel;
    ZeroMemory(&opaquePsoDescPixel, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    opaquePsoDescPixel = opaquePsoDesc;
    opaquePsoDescPixel.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PSPixel"]->GetBufferPointer()),
        mShaders["PSPixel"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDescPixel, IID_PPV_ARGS(&mPSOs["opaquePixel"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc;
    //
    // PSO for debug layer
    //
    ZeroMemory(&debugPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    debugPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    debugPsoDesc.pRootSignature = mRootSignatureDebug.Get();
    debugPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["debugVS"]->GetBufferPointer()),
        mShaders["debugVS"]->GetBufferSize()
    };
    debugPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["debugPS"]->GetBufferPointer()),
        mShaders["debugPS"]->GetBufferSize()
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
    debugPsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["debug"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightPsoDesc;
    ZeroMemory(&lightPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    lightPsoDesc.InputLayout = { mInputLayoutLight.data(), (UINT)mInputLayoutLight.size() };
    lightPsoDesc.pRootSignature = mRootSignatureLight.Get();
    lightPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["DeferredVSLighting"]->GetBufferPointer()),
        mShaders["DeferredVSLighting"]->GetBufferSize()
    };
    lightPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["DeferredPSLighting"]->GetBufferPointer()),
        mShaders["DeferredPSLighting"]->GetBufferSize()
    };
    lightPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    lightPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    lightPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    lightPsoDesc.DepthStencilState.DepthEnable = FALSE;
    lightPsoDesc.DepthStencilState.StencilEnable = FALSE;
    lightPsoDesc.SampleMask = UINT_MAX;
    lightPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    lightPsoDesc.NumRenderTargets = 1;
    lightPsoDesc.RTVFormats[0] = mBackBufferFormat;
    lightPsoDesc.SampleDesc.Count = 1;
    lightPsoDesc.SampleDesc.Quality = 0;
    lightPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&lightPsoDesc, IID_PPV_ARGS(&mPSOs["deferredLighting"])));

    //
    // PSO for billboard sprites
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC billboardSpritePsoDesc;
    ZeroMemory(&billboardSpritePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    billboardSpritePsoDesc.pRootSignature = mRootSignatureBillboard.Get();
    billboardSpritePsoDesc.InputLayout = { mBillboardSpriteInputLayout.data(), (UINT)mBillboardSpriteInputLayout.size() };
    billboardSpritePsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["billboardSpriteVS"]->GetBufferPointer()),
        mShaders["billboardSpriteVS"]->GetBufferSize()
    };
    billboardSpritePsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(mShaders["billboardSpriteGS"]->GetBufferPointer()),
        mShaders["billboardSpriteGS"]->GetBufferSize()
    };
    billboardSpritePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["billboardSpritePS"]->GetBufferPointer()),
        mShaders["billboardSpritePS"]->GetBufferSize()
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
    billboardSpritePsoDesc.DSVFormat = mDepthStencilFormat;
    billboardSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    billboardSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&billboardSpritePsoDesc, IID_PPV_ARGS(&mPSOs["billboardSprites"])));

    //
    // PSO for particles compute
    //
    D3D12_COMPUTE_PIPELINE_STATE_DESC particlesComputePsoDesc;
    ZeroMemory(&particlesComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

    particlesComputePsoDesc.pRootSignature = mRootSignatureParticlesCompute.Get();
    particlesComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(mShaders["particlesCS"]->GetBufferPointer()),
        mShaders["particlesCS"]->GetBufferSize()
    };
    particlesComputePsoDesc.NodeMask = 0;
    particlesComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
   

    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&particlesComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeParticles"])));


    //
    // PSO for particles render
    //
    D3D12_GRAPHICS_PIPELINE_STATE_DESC particlesPsoDesc;
    ZeroMemory(&particlesPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    particlesPsoDesc.pRootSignature = mRootSignatureParticlesRender.Get();
    particlesPsoDesc.InputLayout = { mParticlesInputLayout.data(), (UINT)mParticlesInputLayout.size() };
    particlesPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["particlesVS"]->GetBufferPointer()),
        mShaders["particlesVS"]->GetBufferSize()
    };
    particlesPsoDesc.GS =
    {
        reinterpret_cast<BYTE*>(mShaders["particlesGS"]->GetBufferPointer()),
        mShaders["particlesGS"]->GetBufferSize()
    };
    particlesPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["particlesPS"]->GetBufferPointer()),
        mShaders["particlesPS"]->GetBufferSize()
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
    particlesPsoDesc.DSVFormat = mDepthStencilFormat;
    particlesPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&particlesPsoDesc, IID_PPV_ARGS(&mPSOs["renderParticles"])));

    // Post Processing PSOs
    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingGCPsoDesc;
    ZeroMemory(&postProcessingGCPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    postProcessingGCPsoDesc.InputLayout = { mPostProcessingInputLayout.data(), (UINT)mPostProcessingInputLayout.size() };
    postProcessingGCPsoDesc.pRootSignature = mRootSignaturePostProcessing.Get();
    postProcessingGCPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingVS"]->GetBufferPointer()),
        mShaders["PostProcessingVS"]->GetBufferSize()
    };
    postProcessingGCPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_GC"]->GetBufferPointer()),
        mShaders["PostProcessingPS_GC"]->GetBufferSize()
    };
    postProcessingGCPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    postProcessingGCPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    postProcessingGCPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    postProcessingGCPsoDesc.DepthStencilState.DepthEnable = FALSE;
    postProcessingGCPsoDesc.DepthStencilState.StencilEnable = FALSE;
    postProcessingGCPsoDesc.SampleMask = UINT_MAX;
    postProcessingGCPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    postProcessingGCPsoDesc.NumRenderTargets = 1;
    postProcessingGCPsoDesc.RTVFormats[0] = mBackBufferFormat;
    postProcessingGCPsoDesc.SampleDesc.Count = 1;
    postProcessingGCPsoDesc.SampleDesc.Quality = 0;
    postProcessingGCPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingGCPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_GC"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingGBPsoDesc = postProcessingGCPsoDesc;
    postProcessingGBPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_GB"]->GetBufferPointer()),
        mShaders["PostProcessingPS_GB"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingGBPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_GB"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingCAPsoDesc = postProcessingGCPsoDesc;
    postProcessingCAPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_CA"]->GetBufferPointer()),
        mShaders["PostProcessingPS_CA"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingCAPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_CA"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingVigPsoDesc = postProcessingGCPsoDesc;
    postProcessingVigPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_Vig"]->GetBufferPointer()),
        mShaders["PostProcessingPS_Vig"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingVigPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Vig"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingNoisePsoDesc = postProcessingGCPsoDesc;
    postProcessingNoisePsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_Noise"]->GetBufferPointer()),
        mShaders["PostProcessingPS_Noise"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingNoisePsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Noise"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC postProcessingDefaultPsoDesc = postProcessingGCPsoDesc;
    postProcessingDefaultPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["PostProcessingPS_Default"]->GetBufferPointer()),
        mShaders["PostProcessingPS_Default"]->GetBufferSize()
    };
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&postProcessingDefaultPsoDesc, IID_PPV_ARGS(&mPSOs["postProcessing_Default"])));

    //
   // PSO for noise compute
   //
    D3D12_COMPUTE_PIPELINE_STATE_DESC noiseComputePsoDesc;
    ZeroMemory(&noiseComputePsoDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));

    noiseComputePsoDesc.pRootSignature = mRootSignatureComputeNoise.Get();
    noiseComputePsoDesc.CS = {
        reinterpret_cast<BYTE*>(mShaders["noiseCS"]->GetBufferPointer()),
        mShaders["noiseCS"]->GetBufferSize()
    };
    noiseComputePsoDesc.NodeMask = 0;
    noiseComputePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    ThrowIfFailed(md3dDevice->CreateComputePipelineState(&noiseComputePsoDesc, IID_PPV_ARGS(&mPSOs["computeNoise"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC forwardRTPsoDesc;
    ZeroMemory(&forwardRTPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    forwardRTPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    forwardRTPsoDesc.pRootSignature = mRootSignature.Get();
    forwardRTPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardRT_VS"]->GetBufferPointer()),
        mShaders["forwardRT_VS"]->GetBufferSize()
    };
    forwardRTPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardRT_HS"]->GetBufferPointer()),
        mShaders["forwardRT_HS"]->GetBufferSize()
    };
    forwardRTPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardRT_DS"]->GetBufferPointer()),
        mShaders["forwardRT_DS"]->GetBufferSize()
    };
    forwardRTPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["forwardRT_PS"]->GetBufferPointer()),
        mShaders["forwardRT_PS"]->GetBufferSize()
    };
    forwardRTPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    forwardRTPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    forwardRTPsoDesc.SampleMask = UINT_MAX;
    forwardRTPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    forwardRTPsoDesc.NumRenderTargets = 1;
    forwardRTPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    forwardRTPsoDesc.SampleDesc.Count = 1;
    forwardRTPsoDesc.SampleDesc.Quality = 0;
    forwardRTPsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&forwardRTPsoDesc, IID_PPV_ARGS(&mPSOs["forwardRT"])));


    D3D12_GRAPHICS_PIPELINE_STATE_DESC decalsTessPsoDesc;
    ZeroMemory(&decalsTessPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    decalsTessPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    decalsTessPsoDesc.pRootSignature = mRootSignature.Get();
    decalsTessPsoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mShaders["decalsTessVS"]->GetBufferPointer()),
        mShaders["decalsTessVS"]->GetBufferSize()
    };
    decalsTessPsoDesc.HS =
    {
        reinterpret_cast<BYTE*>(mShaders["decalsTessHS"]->GetBufferPointer()),
        mShaders["decalsTessHS"]->GetBufferSize()
    };
    decalsTessPsoDesc.DS =
    {
        reinterpret_cast<BYTE*>(mShaders["decalsTessDS"]->GetBufferPointer()),
        mShaders["decalsTessDS"]->GetBufferSize()
    };
    decalsTessPsoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mShaders["decalsTessPS"]->GetBufferPointer()),
        mShaders["decalsTessPS"]->GetBufferSize()
    };
    decalsTessPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    decalsTessPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
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
    decalsTessPsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&decalsTessPsoDesc, IID_PPV_ARGS(&mPSOs["decalsTess"])));

    D3D12_GRAPHICS_PIPELINE_STATE_DESC decalsTessWireframePsoDesc;
    ZeroMemory(&decalsTessWireframePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    decalsTessWireframePsoDesc = decalsTessPsoDesc;
    decalsTessWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&decalsTessWireframePsoDesc, IID_PPV_ARGS(&mPSOs["decalsTessWireframe"])));
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
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            2, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
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

    mMaterials["metalAnimate"] = std::move(metalAnimate);

    auto tileCrate = std::make_unique<Material>();
    tileCrate->Name = "tileCrate";
    tileCrate->MatCBIndex = 1;
    tileCrate->DiffuseSrvHeapIndex = 1;
    tileCrate->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    tileCrate->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
    tileCrate->Roughness = 0.2f;

    mMaterials["tileCrate"] = std::move(tileCrate);

    auto stoneMat = std::make_unique<Material>();
    stoneMat->Name = "stoneMaterial";
    stoneMat->MatCBIndex = 2;
    stoneMat->DiffuseSrvHeapIndex = 6;
    stoneMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    stoneMat->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    stoneMat->Roughness = 0.5f;

    mMaterials["stoneMaterial"] = std::move(stoneMat);

    auto planetMat = std::make_unique<Material>();
    planetMat->Name = "planetMaterial";
    planetMat->MatCBIndex = 3;
    planetMat->DiffuseSrvHeapIndex = 17;
    planetMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    planetMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    planetMat->Roughness = 0.125f;

    mMaterials["planetMaterial"] = std::move(planetMat);

    auto particleMat = std::make_unique<Material>();
    particleMat->Name = "particleMaterial";
    particleMat->MatCBIndex = 4;
    particleMat->DiffuseSrvHeapIndex = 8;
    particleMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particleMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particleMat->Roughness = 0.125f;

    mMaterials["particleMaterial"] = std::move(particleMat);

    auto particle2Mat = std::make_unique<Material>();
    particle2Mat->Name = "particle2Material";
    particle2Mat->MatCBIndex = 4;
    particle2Mat->DiffuseSrvHeapIndex = 9;
    particle2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particle2Mat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particle2Mat->Roughness = 0.125f;

    mMaterials["particle2Material"] = std::move(particle2Mat);

    auto skyboxMat = std::make_unique<Material>();
    skyboxMat->Name = "skyboxMaterial";
    skyboxMat->MatCBIndex = 5;
    skyboxMat->DiffuseSrvHeapIndex = 18;
    skyboxMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    skyboxMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    skyboxMat->Roughness = 0.125f;

    mMaterials["skyboxMaterial"] = std::move(skyboxMat);
}

void Engine::BuildRenderItems()
{
    auto boxRitem = std::make_unique<RenderItem>();
    boxRitem->ObjCBIndex = 0;
    boxRitem->World = MathHelper::Identity4x4();
    boxRitem->Mat = mMaterials["metalAnimate"].get();
    boxRitem->Geo = mGeometries["boxGeo"].get();
    boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    mRitemLayer[(int)RenderLayer::Scene10].push_back(boxRitem.get());
    mAllRitems.push_back(std::move(boxRitem));

    auto boxTileRitem = std::make_unique<RenderItem>();
    boxTileRitem->ObjCBIndex = 1;
    XMStoreFloat4x4(&boxTileRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(2.0f, 2.0f, 0.0f));
    boxTileRitem->Mat = mMaterials["tileCrate"].get();
    boxTileRitem->Geo = mGeometries["boxGeo"].get();
    boxTileRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxTileRitem->IndexCount = boxTileRitem->Geo->DrawArgs["box2"].IndexCount;
    boxTileRitem->StartIndexLocation = boxTileRitem->Geo->DrawArgs["box2"].StartIndexLocation;
    boxTileRitem->BaseVertexLocation = boxTileRitem->Geo->DrawArgs["box2"].BaseVertexLocation;
    mRitemLayer[(int)RenderLayer::Scene10].push_back(boxTileRitem.get());
    mAllRitems.push_back(std::move(boxTileRitem));


    auto stoneTesselationRitem = std::make_unique<RenderItem>();
    stoneTesselationRitem->ObjCBIndex = 2;
    XMStoreFloat4x4(&stoneTesselationRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f));
    stoneTesselationRitem->TexTransform = MathHelper::Identity4x4();
    stoneTesselationRitem->Mat = mMaterials["stoneMaterial"].get();
    stoneTesselationRitem->Geo = mGeometries["boxGeo"].get();
    stoneTesselationRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    stoneTesselationRitem->IndexCount = stoneTesselationRitem->Geo->DrawArgs["box3"].IndexCount;
    stoneTesselationRitem->StartIndexLocation = stoneTesselationRitem->Geo->DrawArgs["box3"].StartIndexLocation;
    stoneTesselationRitem->BaseVertexLocation = stoneTesselationRitem->Geo->DrawArgs["box3"].BaseVertexLocation;
    mRitemLayer[(int)RenderLayer::Scene10].push_back(stoneTesselationRitem.get());
    mRitemLayer[(int)RenderLayer::Scene11].push_back(stoneTesselationRitem.get());
    mAllRitems.push_back(std::move(stoneTesselationRitem));


    auto boxInstancingRitem = std::make_unique<RenderItem>();
    boxInstancingRitem->ObjCBIndex = 3;
    boxInstancingRitem->World = MathHelper::Identity4x4();
    boxInstancingRitem->Mat = mMaterials["stoneMaterial"].get();
    boxInstancingRitem->Geo = mGeometries["boxGeo"].get();
    boxInstancingRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxInstancingRitem->IndexCount = boxInstancingRitem->Geo->DrawArgs["box4"].IndexCount;
    boxInstancingRitem->StartIndexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].StartIndexLocation;
    boxInstancingRitem->BaseVertexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].BaseVertexLocation;
    mAllRitems.push_back(std::move(boxInstancingRitem));

    auto boxFPSRitem = std::make_unique<RenderItem>();
    boxFPSRitem->ObjCBIndex = 4;
    boxFPSRitem->World = MathHelper::Identity4x4();
    boxFPSRitem->Mat = mMaterials["stoneMaterial"].get();
    boxFPSRitem->Geo = mGeometries["boxGeo"].get();
    boxFPSRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxFPSRitem->IndexCount = boxFPSRitem->Geo->DrawArgs["boxFPS"].IndexCount;
    boxFPSRitem->StartIndexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].StartIndexLocation;
    boxFPSRitem->BaseVertexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].BaseVertexLocation;
    mAllRitems.push_back(std::move(boxFPSRitem));

    auto pointLight1Ritem = std::make_unique<RenderItem>();
    pointLight1Ritem->ObjCBIndex = 5;
    pointLight1Ritem->World = MathHelper::Identity4x4();
    pointLight1Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight1Ritem->Mat = mMaterials["stoneMaterial"].get();
    pointLight1Ritem->Geo = mGeometries["boxGeo"].get();
    pointLight1Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight1Ritem->IndexCount = pointLight1Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    pointLight1Ritem->StartIndexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    pointLight1Ritem->BaseVertexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;
    mAllRitems.push_back(std::move(pointLight1Ritem));

    auto pointLight2Ritem = std::make_unique<RenderItem>();
    pointLight2Ritem->ObjCBIndex = 6;
    pointLight2Ritem->World = MathHelper::Identity4x4();
    pointLight2Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight2Ritem->Mat = mMaterials["stoneMaterial"].get();
    pointLight2Ritem->Geo = mGeometries["boxGeo"].get();
    pointLight2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight2Ritem->IndexCount = pointLight2Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    pointLight2Ritem->StartIndexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    pointLight2Ritem->BaseVertexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;
    mAllRitems.push_back(std::move(pointLight2Ritem));

    auto pointLight3Ritem = std::make_unique<RenderItem>();
    pointLight3Ritem->ObjCBIndex = 7;
    pointLight3Ritem->World = MathHelper::Identity4x4();
    pointLight3Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight3Ritem->Mat = mMaterials["stoneMaterial"].get();
    pointLight3Ritem->Geo = mGeometries["boxGeo"].get();
    pointLight3Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight3Ritem->IndexCount = pointLight3Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    pointLight3Ritem->StartIndexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    pointLight3Ritem->BaseVertexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;
    mAllRitems.push_back(std::move(pointLight3Ritem));

    auto gridRitem = std::make_unique<RenderItem>();
    gridRitem->ObjCBIndex = 8;
    gridRitem->World = MathHelper::Identity4x4();
    gridRitem->TexTransform = MathHelper::Identity4x4();
    gridRitem->Mat = mMaterials["stoneMaterial"].get();
    gridRitem->Geo = mGeometries["boxGeo"].get();
    gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    mAllRitems.push_back(std::move(gridRitem));

    auto coneRitem = std::make_unique<RenderItem>();
    coneRitem->ObjCBIndex = 9;
    coneRitem->World = MathHelper::Identity4x4();
    coneRitem->TexTransform = MathHelper::Identity4x4();
    coneRitem->Mat = mMaterials["stoneMaterial"].get();
    coneRitem->Geo = mGeometries["boxGeo"].get();
    coneRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    coneRitem->IndexCount = coneRitem->Geo->DrawArgs["cone"].IndexCount;
    coneRitem->StartIndexLocation = coneRitem->Geo->DrawArgs["cone"].StartIndexLocation;
    coneRitem->BaseVertexLocation = coneRitem->Geo->DrawArgs["cone"].BaseVertexLocation;
    mAllRitems.push_back(std::move(coneRitem));

    auto planetSpritesRitem = std::make_unique<RenderItem>();
    planetSpritesRitem->World = MathHelper::Identity4x4();
    planetSpritesRitem->ObjCBIndex = 10;
    planetSpritesRitem->Mat = mMaterials["planetMaterial"].get();
    planetSpritesRitem->Geo = mGeometries["planetSpritesGeo"].get();
    planetSpritesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    planetSpritesRitem->IndexCount = planetSpritesRitem->Geo->DrawArgs["points"].IndexCount;
    planetSpritesRitem->StartIndexLocation = planetSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
    planetSpritesRitem->BaseVertexLocation = planetSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::BillboardSprites].push_back(planetSpritesRitem.get());
    mAllRitems.push_back(std::move(planetSpritesRitem));

    auto particleRitem = std::make_unique<RenderItem>();
    particleRitem->World = MathHelper::Identity4x4();
    particleRitem->ObjCBIndex = 11;
    particleRitem->Mat = mMaterials["particleMaterial"].get();
    particleRitem->Geo = mGeometries["particlesGeo1"].get();
    particleRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particleRitem->IndexCount = particleRitem->Geo->DrawArgs["points1"].IndexCount;
    particleRitem->StartIndexLocation = particleRitem->Geo->DrawArgs["points1"].StartIndexLocation;
    particleRitem->BaseVertexLocation = particleRitem->Geo->DrawArgs["points1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles1].push_back(particleRitem.get());
    mAllRitems.push_back(std::move(particleRitem));

    auto skyboxRitem = std::make_unique<RenderItem>();
    skyboxRitem->World = MathHelper::Identity4x4();
    skyboxRitem->ObjCBIndex = 12;
    skyboxRitem->Mat = mMaterials["skyboxMaterial"].get();
    skyboxRitem->Geo = mGeometries["boxGeo"].get();
    skyboxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    skyboxRitem->IndexCount = skyboxRitem->Geo->DrawArgs["skybox"].IndexCount;
    skyboxRitem->StartIndexLocation = skyboxRitem->Geo->DrawArgs["skybox"].StartIndexLocation;
    skyboxRitem->BaseVertexLocation = skyboxRitem->Geo->DrawArgs["skybox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene2Opaque].push_back(skyboxRitem.get());
    mAllRitems.push_back(std::move(skyboxRitem));

    auto instancingRitemScene3 = std::make_unique<RenderItem>();
    instancingRitemScene3->World = MathHelper::Identity4x4();
    instancingRitemScene3->ObjCBIndex = 13;
    instancingRitemScene3->Mat = mMaterials["stoneMaterial"].get();
    instancingRitemScene3->Geo = mGeometries["scene3Geo"].get();
    instancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    instancingRitemScene3->IndexCount = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
    instancingRitemScene3->StartIndexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
    instancingRitemScene3->BaseVertexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;
    instancingRitemScene3->Bounds = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].Bounds;

    mRitemLayer[(int)RenderLayer::Scene3].push_back(instancingRitemScene3.get());
    mAllRitems.push_back(std::move(instancingRitemScene3));

    for (int i = 0; i < 729; ++i)
    {
        auto nonInstancingRitemScene3 = std::make_unique<RenderItem>();
        nonInstancingRitemScene3->World = MathHelper::Identity4x4();
        nonInstancingRitemScene3->ObjCBIndex = 14 + i;
        nonInstancingRitemScene3->Mat = mMaterials["stoneMaterial"].get();
        nonInstancingRitemScene3->Geo = mGeometries["scene3Geo"].get();
        nonInstancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        nonInstancingRitemScene3->IndexCount = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
        nonInstancingRitemScene3->StartIndexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
        nonInstancingRitemScene3->BaseVertexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;

        mRitemLayer[(int)RenderLayer::Scene3].push_back(nonInstancingRitemScene3.get());
        mAllRitems.push_back(std::move(nonInstancingRitemScene3));
    }

    auto animateRitemScene4 = std::make_unique<RenderItem>();
    animateRitemScene4->World = MathHelper::Identity4x4();
    animateRitemScene4->ObjCBIndex = 743;
    animateRitemScene4->Mat = mMaterials["metalAnimate"].get();
    animateRitemScene4->Geo = mGeometries["scene4Geo"].get();
    animateRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    animateRitemScene4->IndexCount = animateRitemScene4->Geo->DrawArgs["animateBox"].IndexCount;
    animateRitemScene4->StartIndexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].StartIndexLocation;
    animateRitemScene4->BaseVertexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(animateRitemScene4.get());
    mAllRitems.push_back(std::move(animateRitemScene4));

    auto tilingRitemScene4 = std::make_unique<RenderItem>();
    tilingRitemScene4->World = MathHelper::Identity4x4();
    tilingRitemScene4->ObjCBIndex = 744;
    tilingRitemScene4->Mat = mMaterials["tileCrate"].get();
    tilingRitemScene4->Geo = mGeometries["scene4Geo"].get();
    tilingRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tilingRitemScene4->IndexCount = tilingRitemScene4->Geo->DrawArgs["tilingBox"].IndexCount;
    tilingRitemScene4->StartIndexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].StartIndexLocation;
    tilingRitemScene4->BaseVertexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(tilingRitemScene4.get());
    mAllRitems.push_back(std::move(tilingRitemScene4));

    auto tesselationRitemScene5 = std::make_unique<RenderItem>();
    tesselationRitemScene5->World = MathHelper::Identity4x4();
    tesselationRitemScene5->ObjCBIndex = 745;
    tesselationRitemScene5->Mat = mMaterials["stoneMaterial"].get();
    tesselationRitemScene5->Geo = mGeometries["scene5Geo"].get();
    tesselationRitemScene5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tesselationRitemScene5->IndexCount = tesselationRitemScene5->Geo->DrawArgs["tesselationBox"].IndexCount;
    tesselationRitemScene5->StartIndexLocation = tesselationRitemScene5->Geo->DrawArgs["tesselationBox"].StartIndexLocation;
    tesselationRitemScene5->BaseVertexLocation = tesselationRitemScene5->Geo->DrawArgs["tesselationBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene5].push_back(tesselationRitemScene5.get());
    mAllRitems.push_back(std::move(tesselationRitemScene5));

    auto particle2Ritem = std::make_unique<RenderItem>();
    particle2Ritem->World = MathHelper::Identity4x4();
    particle2Ritem->ObjCBIndex = 746;
    particle2Ritem->Mat = mMaterials["particle2Material"].get();
    particle2Ritem->Geo = mGeometries["particlesGeo2"].get();
    particle2Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particle2Ritem->IndexCount = particle2Ritem->Geo->DrawArgs["points2"].IndexCount;
    particle2Ritem->StartIndexLocation = particle2Ritem->Geo->DrawArgs["points2"].StartIndexLocation;
    particle2Ritem->BaseVertexLocation = particle2Ritem->Geo->DrawArgs["points2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles2].push_back(particle2Ritem.get());
    mAllRitems.push_back(std::move(particle2Ritem));

    for (int i = 0; i < mAllRitems.size(); ++i)
    {
        if (i < 5)
            mRitemLayer[(int)RenderLayer::Opaque].push_back(mAllRitems[i].get());
        else if (i < 10)
            mRitemLayer[(int)RenderLayer::OpaqueWireframe].push_back(mAllRitems[i].get());
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

    mGeometries["planetSpritesGeo"] = std::move(geo);
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


    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    md3dDevice->CreateDepthStencilView(mDepthStencilBufferScene3.Get(), &dsvDesc, DepthStencilViewScene3());

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

    D3D12_SUBRESOURCE_DATA instanceDataSub = {
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

void Engine::InitParticleSystem()
{
    mParticleSystem = new ParticleSystem(64, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 1);

    mParticleSystem->InitializeSystem(md3dDevice,
        particleBuffers,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize,
        0);

    mParticleSystem->BuildSystemVertexBuffers(mGeometries,
        md3dDevice,
        mCommandList);


    mParticleSystem2 = new ParticleSystem(512, DirectX::XMFLOAT3(0.f, 0.f, 0.f), 2);

    mParticleSystem2->InitializeSystem(md3dDevice,
        particle2Buffers,
        mParticlesSrvUavHeap,
        mCbvSrvUavDescriptorSize, 
        1);

    mParticleSystem2->BuildSystemVertexBuffers(mGeometries,
        md3dDevice,
        mCommandList);
}

void Engine::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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
            auto instanceBuffer = mCurrFrameResource->InstancingCB->Resource();

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

void Engine::DrawRenderItemsScene4(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

void Engine::DrawRenderItemsScene10(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

void Engine::DrawRenderItemsScene11(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

void Engine::DrawDebugRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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

    auto objectCB = mCurrFrameResource->ObjectCB->Resource();
    auto matCB = mCurrFrameResource->MaterialCB->Resource();

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
    auto geo = mGeometries["screenQuad"].get();
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
    auto geo = mGeometries["screenQuad"].get();
    auto tmp1 = geo->VertexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &tmp1);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    CD3DX12_GPU_DESCRIPTOR_HANDLE srv(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
    srv.Offset(20, mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srv);

    cmdList->DrawInstanced(4, 1, 0, 0);
}

void Engine::DrawParticles(ParticleSystem particleSystem, RenderLayer layer)
{
    particleSystem.Render(mCommandList.Get(),
        mRitemLayer[(int)layer],
        mCbvSrvDescriptorSize,
        mCurrFrameResource,
        mParticlesSrvUavHeap);
}


std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Engine::GetStaticSamplers()
{
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.  

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        8);                                // maxAnisotropy

    return {
        pointWrap, pointClamp,
        linearWrap, linearClamp,
        anisotropicWrap, anisotropicClamp };
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> Engine::GetMoreStaticSamplers()
{
    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        2, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointBorder(
        4, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointMirror(
        5, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointMirrorOnce(
        6, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE); // addressW

    return { pointWrap, linearWrap, anisotropicWrap, pointClamp, pointBorder, pointMirror, pointMirrorOnce };
}

std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> Engine::GetLODStaticSamplers()
{
    CD3DX12_STATIC_SAMPLER_DESC linearWrap0(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
    linearWrap0.MinLOD = 3;

    CD3DX12_STATIC_SAMPLER_DESC linearWrap1(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
    linearWrap1.MinLOD = 2;

    CD3DX12_STATIC_SAMPLER_DESC linearWrap2(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
    linearWrap2.MinLOD = 1;

    CD3DX12_STATIC_SAMPLER_DESC linearWrap3(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW
    linearWrap3.MinLOD = 0;

    return { linearWrap0, linearWrap1, linearWrap2, linearWrap3 };
}

void Engine::RenderUI()
{
    ImVec2 infoPanelSize = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
    ImVec2 infoPanelPos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(infoPanelPos);
    ImGui::SetNextWindowSize(infoPanelSize);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Info Panel", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("Scene 1: Sandbox");
        ImGui::Text("Scene 2: Day/night scene");
        ImGui::Text("Scene 3: Instancing/frumstum culling scene");
        ImGui::Text("Scene 4: Texture animation & Tiling scene");
        ImGui::Text("Scene 5: Tesselation scene");
        ImGui::Text("Scene 6: Particles scene");
        ImGui::Text("Scene 7: Shadows scene");
        ImGui::Text("Scene 8: Post-processing scene");
        ImGui::Text("Scene 9: PBR scene");
        ImGui::Text("Scene 10: Rendering Techniques");
        ImGui::Text("Scene 11: Decals Tesselation");
    } ImGui::End();

    ImVec2 scenePanelSize = ImVec2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 10);
    ImVec2 scenePanelPos = ImVec2(WINDOW_WIDTH / 2 - scenePanelSize.x / 2, WINDOW_HEIGHT - scenePanelSize.y);
    ImGui::SetNextWindowPos(scenePanelPos);
    ImGui::SetNextWindowSize(scenePanelSize);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Scene Selector", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        if (ImGui::BeginTable("Scenes", 8))
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Button("Scene 1", ImVec2(80, 40)))
            {
                activeSceneID = 1;
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Scene 2", ImVec2(80, 40)))
            {
                activeSceneID = 2;
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("Scene 3", ImVec2(80, 40)))
            {
                activeSceneID = 3;
            }

            ImGui::TableSetColumnIndex(3);
            if (ImGui::Button("Scene 4", ImVec2(80, 40)))
            {
                activeSceneID = 4;
            }

            ImGui::TableSetColumnIndex(4);
            if (ImGui::Button("Scene 5", ImVec2(80, 40)))
            {
                activeSceneID = 5;
            }

            ImGui::TableSetColumnIndex(5);
            if (ImGui::Button("Scene 6", ImVec2(80, 40)))
            {
                activeSceneID = 6;
            }

            ImGui::TableSetColumnIndex(6);
            if (ImGui::Button("Scene 7", ImVec2(80, 40)))
            {
                activeSceneID = 7;
            }

            ImGui::TableSetColumnIndex(7);
            if (ImGui::Button("Scene 8", ImVec2(80, 40)))
            {
                activeSceneID = 8;
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Button("Scene 9", ImVec2(80, 40)))
            {
                activeSceneID = 9;
            }

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button("Scene 10", ImVec2(80, 40)))
            {
                activeSceneID = 10;
            }
            ImGui::TableSetColumnIndex(2);

            if (ImGui::Button("Scene 11", ImVec2(80, 40)))
            {
                activeSceneID = 11;
            }

            ImGui::EndTable();
        }
    } ImGui::End();

    ImVec2 size = ImVec2(WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4 * 3);
    ImVec2 pos = ImVec2(WINDOW_WIDTH - size.x, 0);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Configuration", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

        if (activeSceneID == 1)
        {
            ImGui::Text("");
            ImGui::Text("Deferred Render");
            ImGui::Checkbox("Deferred Render Info", &deferredRenderDisplayInfo);
        }

        if (activeSceneID == 1)
        {
            ImGui::Text("");
            ImGui::Checkbox("Debug Layer", &isDebug);
            ImGui::Checkbox("Debug Grid", &gridIsActive);
            ImGui::Text("");

            ImGui::Checkbox("FPS Object", &fpsObjectIsActive);
            ImGui::Text("");

            ImGui::Text("Flashlight");
            ImGui::Checkbox("Is Active", &flashlightIsActive);
            ImGui::SliderFloat("Distance Flashlight", &light2SpotDistance, 0.0f, 20.0f);
            ImGui::SliderFloat("Strength Flashlight", &light2SpotStrength, 0.0f, 5.0f);
            ImGui::ColorPicker3("Color Flashlight", colSpot2, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Parallax Mapping");
            ImGui::Checkbox("Parallax Mapping", &isParallaxMapping);

            ImGui::Text("");
            ImGui::Text("Tesselation & Displacement");
            ImGui::Checkbox("Solid Mode", &isSolid);
            ImGui::SliderFloat("Tesselation Factor", &tessFactor, 1.f, 64.f);
            ImGui::SliderFloat("Displacement Level", &displacementLevel, 0.f, 5.f);

            ImGui::Text("");
            ImGui::Text("Effects");
            ImGui::Checkbox("Negative", &isNegative);
            ImGui::Checkbox("Pixelation Shader", &isPixelated);
            ImGui::SliderInt("Pixelated Factor", &pixelationFactor, 16.f, 128.f);

            ImGui::Text("");
            ImGui::Text("Instancing");
            ImGui::SliderInt("Instancing Level", &instancingLevel, 1.f, 30.f);

            ImGui::Text("");
            if (ImGui::Button("Object 1", ImVec2(100, 40)))
            {
                if (selectedObjectID == 1)
                    selectedObjectID = 0;
                else selectedObjectID = 1;
            }
            if (ImGui::Button("Object 2", ImVec2(100, 40)))
            {
                if (selectedObjectID == 2)
                    selectedObjectID = 0;
                else selectedObjectID = 2;
            }

            mAllRitems[0]->NumFramesDirty = 1;
            mAllRitems[1]->NumFramesDirty = 1;
            mAllRitems[4]->NumFramesDirty = 1;
            mAllRitems[5]->NumFramesDirty = 1;
            mAllRitems[6]->NumFramesDirty = 1;
            mAllRitems[9]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 2)
        {
            ImGui::Text("");
            ImGui::Text("Time: %s", timeScene2.c_str());
        }
        else if (activeSceneID == 3)
        {
            ImGui::Text("");
            ImGui::Text("Frustum Culling");
            ImGui::Checkbox("Frustum Culling Active", &isFrustumCullingScene3);
            ImGui::Checkbox("Frustum Culling Info", &isDisplayingFrustumCullingInfoScene3);

            ImGui::Text("");
            ImGui::Text("Instancing");
            ImGui::Checkbox("Instancing Active", &isUsingInstancingScene3);

            ImGui::Text("");
            ImGui::SliderInt("Level Of Details", &levelOfDetailsScene3, 0, 3);
        }
        else if (activeSceneID == 4)
        {
            ImGui::Checkbox("Animate Material", &isAnimateMaterialScene4);
            ImGui::SliderInt("Tiles Count", &tilesCountScene4, 1, 6);
            ImGui::Text("Filtering Mode");
            ImGui::RadioButton("Point", &filteringModeScene4, 0);
            ImGui::RadioButton("Linear", &filteringModeScene4, 1);
            ImGui::RadioButton("Anisotrophic", &filteringModeScene4, 2);
            if (filteringModeScene4 == 0)
            {
                ImGui::Text("Address Mode");
                ImGui::RadioButton("Wrap", &addressModeScene4, 0);
                ImGui::RadioButton("Clamp", &addressModeScene4, 1);
                ImGui::RadioButton("Border", &addressModeScene4, 2);
                ImGui::RadioButton("Mirror", &addressModeScene4, 3);
                ImGui::RadioButton("Mirror Once", &addressModeScene4, 4);
            }
            mAllRitems[743]->NumFramesDirty = 1;
            mAllRitems[744]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 5)
        {
            ImGui::Text("");
            ImGui::Text("Tesselation & Displacement");
            ImGui::Checkbox("Solid Mode", &isSolidScene5);
            ImGui::SliderFloat("Tesselation Factor", &tessFactorScene5, 1.f, 64.f);
            ImGui::SliderFloat("Displacement Level", &displacementLevelScene5, 0.f, 5.f);
            mAllRitems[745]->NumFramesDirty = 1;
        }
        else if (activeSceneID == 6)
        {
            ImGui::Text("");
            ImGui::Text("Particles");
            if (ImGui::BeginTable("Systems", 2))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Button("System 1", ImVec2(100, 40)))
                {
                    activeParticleSystemScene6 = 1;
                }

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("System 2", ImVec2(100, 40)))
                {
                    activeParticleSystemScene6 = 2;
                }

                ImGui::EndTable();
            }

            if (activeParticleSystemScene6 == 1)
            {
                ImGui::Text("");
                ImGui::Checkbox("Particles", &particles1IsActive);
                ImGui::Text("Particle System Settings");
                ImGui::SliderFloat("Particle Start Size", &particleStartSizeScene6, 0.5f, 8.f);
                ImGui::SliderFloat("Particle End Size", &particleEndSizeScene6, 0.0f, 0.5f);
                ImGui::ColorPicker3("Particle Start Color", particleStartColorScene6, ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorPicker3("Particle End Color", particleEndColorScene6, ImGuiColorEditFlags_NoAlpha);

                mParticleSystem->emitterData.StartColor = DirectX::XMFLOAT4(particleStartColorScene6[0], particleStartColorScene6[1], particleStartColorScene6[2], 1.0f);
                mParticleSystem->emitterData.EndColor = DirectX::XMFLOAT4(particleEndColorScene6[0], particleEndColorScene6[1], particleEndColorScene6[2], 1.0f);
                mParticleSystem->emitterData.StartSize = particleStartSizeScene6;
                mParticleSystem->emitterData.EndSize = particleEndSizeScene6;
            }
            else if (activeParticleSystemScene6 == 2)
            {
                ImGui::Text("");
                ImGui::Checkbox("Particles", &particles2IsActive);
                ImGui::Text("Particle System Settings");
                ImGui::SliderFloat("Particle Start Size", &particle2StartSizeScene6, 0.5f, 8.f);
                ImGui::SliderFloat("Particle End Size", &particle2EndSizeScene6, 0.0f, 0.5f);
                ImGui::ColorPicker3("Particle Start Color", particle2StartColorScene6, ImGuiColorEditFlags_NoAlpha);
                ImGui::ColorPicker3("Particle End Color", particle2EndColorScene6, ImGuiColorEditFlags_NoAlpha);

                mParticleSystem2->emitterData.StartColor = DirectX::XMFLOAT4(particle2StartColorScene6[0], particle2StartColorScene6[1], particle2StartColorScene6[2], 1.0f);
                mParticleSystem2->emitterData.EndColor = DirectX::XMFLOAT4(particle2EndColorScene6[0], particle2EndColorScene6[1], particle2EndColorScene6[2], 1.0f);
                mParticleSystem2->emitterData.StartSize = particle2StartSizeScene6;
                mParticleSystem2->emitterData.EndSize = particle2EndSizeScene6;
            }
        }
        else if (activeSceneID == 7)
        {

        }
        else if (activeSceneID == 8)
        {
            ImGui::Text("");
            ImGui::Text("Post Processing Effects");
            ImGui::RadioButton("Default", &selectedEffectScene8, 0);
            ImGui::RadioButton("Gamma Correction", &selectedEffectScene8, 1);
            ImGui::RadioButton("Gaussian Blur", &selectedEffectScene8, 2);
            ImGui::RadioButton("Chromatic Aberration", &selectedEffectScene8, 3);
            ImGui::RadioButton("Vignette", &selectedEffectScene8, 4);
            ImGui::RadioButton("Noise", &selectedEffectScene8, 5);

            isActiveNormalScene8 = false;
            isActiveGCScene8 = false;
            isActiveGBScene8 = false;
            isActiveCAScene8 = false;
            isActiveVigScene8 = false;
            isActiveNoiseScene8 = false;
            if (selectedEffectScene8 == 0)
            {
                isActiveNormalScene8 = true;
            }
            else if (selectedEffectScene8 == 1)
            {
                isActiveGCScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Gamma Ratio", &gammaRatioScene8, 0.2f, 5.0f);
            }
            else if (selectedEffectScene8 == 2)
            {
                isActiveGBScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::Checkbox("Horizontal/Vertical", &gbIsHorizontalScene8);
            }
            else if (selectedEffectScene8 == 3)
            {
                isActiveCAScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("DistortionX", &caDistortionScene8.x, 0.0f, 0.1f);
                ImGui::SliderFloat("DistortionY", &caDistortionScene8.y, 0.0f, 0.1f);
                ImGui::SliderFloat("Direction", &caDirectionScene8.x, 0.0f, 1.0f);
                caDirectionScene8.y = sqrt(1.f - caDirectionScene8.x * caDirectionScene8.x);
                ImGui::SliderFloat("Intensity", &caIntensityScene8, 0.0f, 2.0f);
            }
            else if (selectedEffectScene8 == 4)
            {
                isActiveVigScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Intensity", &vIntensityScene8, 0.5f, 1.5f);
                ImGui::SliderFloat("Smoothness", &vSmoothnessScene8, 0.3f, 1.0f);
                ImGui::SliderFloat("Roundness", &vRoundnessScene8, 0.5f, 1.0f);
            }
            else if (selectedEffectScene8 == 5)
            {
                isActiveNoiseScene8 = true;
                ImGui::Text("");
                ImGui::Text("Effect Settings");
                ImGui::SliderFloat("Intensity", &nIntensityScene8, 0.0f, 3.f);
                ImGui::SliderFloat("Size", &nSizeScene8, 0.5f, 5.f);
            }
        }
        else if (activeSceneID == 9)
        {

        }
        else if (activeSceneID == 10)
        {
            ImGui::Text("");
            ImGui::Text("Rendering Techniques");
            ImGui::RadioButton("Forward", &selectedRenderTechScene10, 0);
            ImGui::RadioButton("Deferred", &selectedRenderTechScene10, 1);
            ImGui::RadioButton("Deferred+", &selectedRenderTechScene10, 2);
            ImGui::RadioButton("Forward+", &selectedRenderTechScene10, 3);

            if (selectedRenderTechScene10 == 1 || selectedRenderTechScene10 == 2)
            {
                ImGui::Text("");
                ImGui::Text("Other settings");
                ImGui::Checkbox("Deferred Render Info", &deferredRenderDisplayInfoScene10);
            }
        }
        else if (activeSceneID == 11)
        {
            ImGui::Text("");
            ImGui::Checkbox("Wireframe", &isWireframeScene10);
            ImGui::Text("");
            ImGui::Text("Decals settings");
        }
    } ImGui::End();

    if (deferredRenderDisplayInfo && activeSceneID <= 2)
    {
        ImVec2 size2 = ImVec2(WINDOW_WIDTH / 5, 800);
        ImVec2 pos2 = ImVec2(0, 0);

        ImGui::SetNextWindowPos(pos2);
        ImGui::SetNextWindowSize(size2);
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Deferred Render Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(11, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
        } ImGui::End();
    }

    if (deferredRenderDisplayInfoScene10 && activeSceneID == 10 && selectedRenderTechScene10 != 0)
    {
        ImVec2 size2 = ImVec2(WINDOW_WIDTH / 5, 800);
        ImVec2 pos2 = ImVec2(0, 0);

        ImGui::SetNextWindowPos(pos2);
        ImGui::SetNextWindowSize(size2);
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Deferred Render Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(11, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
            tex.Offset(1, mCbvSrvDescriptorSize);
            ImGui::Image((ImTextureID)tex.ptr, ImVec2((float)240, (float)150));
        } ImGui::End();
    }

    if (activeSceneID == 1)
    {
        ImVec2 lightPanelSize = ImVec2(350.f, 350.f);
        ImGui::SetNextWindowSize(lightPanelSize);
        if (ImGui::Begin("Point Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Point Light 1");
            if (ImGui::BeginTable("PointLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos1X", &lightPos1[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos1Y", &lightPos1[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos1Z", &lightPos1[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 1", &light1Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 1", &light1Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 1", col1, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 2");
            if (ImGui::BeginTable("PointLight2", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos2X", &lightPos2[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos2Y", &lightPos2[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos2Z", &lightPos2[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 2", &light2Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 2", &light2Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 2", col2, ImGuiColorEditFlags_NoAlpha);

            ImGui::Text("");
            ImGui::Text("Point Light 3");
            if (ImGui::BeginTable("PointLight3", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##PointLightPos3X", &lightPos3[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##PointLightPos3Y", &lightPos3[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##PointLightPos3Z", &lightPos3[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("Distance 3", &light3Distance, 0.0f, 2.0f);
            ImGui::SliderFloat("Strength 3", &light3Strength, 0.0f, 1.0f);
            ImGui::ColorPicker3("Color 3", col3, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();

        ImVec2 spotLightPanelSize = ImVec2(350.f, 350.f);
        ImVec2 spotLightPanelPos = ImVec2(100.f, 200.f);
        ImGui::SetNextWindowSize(spotLightPanelSize);
        ImGui::SetNextWindowPos(spotLightPanelPos);
        if (ImGui::Begin("Spot Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("Spot Light 1");
            if (ImGui::BeginTable("SpotLight1", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Pos X");

                ImGui::TableSetColumnIndex(0);
                ImGui::InputFloat("##SpotLightPos1X", &lightPosSpot1[0], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Pos Y");

                ImGui::TableSetColumnIndex(1);
                ImGui::InputFloat("##SpotLightPos1Y", &lightPosSpot1[1], 0.1f, 0.1f);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("Pos Z");

                ImGui::TableSetColumnIndex(2);
                ImGui::InputFloat("##SpotLightPos1Z", &lightPosSpot1[2], 0.1f, 0.1f);
                ImGui::EndTable();
            }
            ImGui::SliderFloat("SpotLightDir1X", &spotLight1Direction[0], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Y", &spotLight1Direction[1], -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("SpotLightDir1Z", &spotLight1Direction[2], -DirectX::XM_PI, DirectX::XM_PI);

            ImGui::SliderFloat("Spot Distance 1", &light1SpotDistance, 0.0f, 20.0f);
            ImGui::SliderFloat("Spot Strength 1", &light1SpotStrength, 0.0f, 10.0f);
            ImGui::ColorPicker3("Spot Color 1", colSpot1, ImGuiColorEditFlags_NoAlpha);

        } ImGui::End();

        // Selected object info
        {
            ImVec2 size3 = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
            ImVec2 pos3 = ImVec2(WINDOW_WIDTH / 4 * 3, WINDOW_HEIGHT / 4 * 3);

            ImGui::SetNextWindowPos(pos3);
            ImGui::SetNextWindowSize(size3);
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Object Settings", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
            {
                switch (selectedObjectID)
                {
                case 1:
                    ImGui::Text("Object 1");
                    if (ImGui::BeginTable("Object1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pos X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##PosXObj1", &Obj1posX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Pos Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##PosYObj1", &Obj1posY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Pos Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##PosZObj1", &Obj1posZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    if (ImGui::BeginTable("Object1_1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rot X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##RotXObj1", &Obj1rotX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Rot Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##RotYObj1", &Obj1rotY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Rot Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##RotZObj1", &Obj1rotZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    ImGui::SliderFloat("Scale", &Obj1Scale, 0.1f, 5.0f);
                    ImGui::Checkbox("Animate Material", &isAnimateMaterial);
                    break;

                case 2:
                    ImGui::Text("Object 2");
                    if (ImGui::BeginTable("Object2", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Pos X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##PosXObj2", &Obj2posX, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Pos Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##PosYObj2", &Obj2posY, 0.1f, 0.1f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Pos Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##PosZObj2", &Obj2posZ, 0.1f, 0.1f);
                        ImGui::EndTable();
                    }
                    if (ImGui::BeginTable("Object2_1", 3))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Rot X");

                        ImGui::TableSetColumnIndex(0);
                        ImGui::InputFloat("##RotXObj2", &Obj2rotX, 1.f, 1.f);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Rot Y");

                        ImGui::TableSetColumnIndex(1);
                        ImGui::InputFloat("##RotYObj2", &Obj2rotY, 1.f, 1.f);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Rot Z");

                        ImGui::TableSetColumnIndex(2);
                        ImGui::InputFloat("##RotZObj2", &Obj2rotZ, 1.f, 1.f);
                        ImGui::EndTable();
                    }
                    ImGui::SliderInt("Tiles Count", &tilesCountInt, 1, 6);
                    break;
                }
            } ImGui::End();
        }

        ChangeTileObjectTiles();
    }
    else if (activeSceneID == 2)
    {
        
    }
    else if (activeSceneID == 3)
    {
        if (isDisplayingFrustumCullingInfoScene3 && isUsingInstancingScene3)
        {
            ImVec2 size = ImVec2(WINDOW_WIDTH / 4, WINDOW_HEIGHT / 3);
            ImVec2 pos = ImVec2(WINDOW_WIDTH / 2 - size.x / 2, 0);

            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            ImGui::SetNextWindowBgAlpha(0.5f);
            if (ImGui::Begin("Frustum Culling Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
                hDescriptor.Offset(19, mCbvSrvDescriptorSize);
                ImGui::Image((ImTextureID)hDescriptor.ptr, ImVec2((float)304, (float)225));
            } ImGui::End();
        }
    }

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());
}
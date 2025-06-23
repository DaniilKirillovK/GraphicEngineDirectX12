#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <vector>

#include "ModelHelper.h"

#include "MathHelper.h"
#include "D3D12Engine.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"
#include "Camera.h"
#include "GBuffer.h"
#include "Model.h"

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

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
    RenderItem() = default;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    // Dirty flag indicating the object data has changed and we need to update the constant buffer.
    // Because we have an object cbuffer for each FrameResource, we have to apply the
    // update to each FrameResource.  Thus, when we modify obect data we should set 
    // NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
    int NumFramesDirty = gNumFrameResources;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

    // DrawInstanced parameters
    UINT VertexCount = 0;
};

struct VertexLightStage 
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
};

enum class RenderLayer : int
{
    Opaque = 0,
    OpaqueWireframe = 1,
    Count
};


struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                      FenceValue;
};

ExampleDescriptorHeapAllocator mSrvHeapAllocator;


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
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

    void ChangeTileObjectTiles();

    virtual void BuildDescriptorHeaps() override;
    virtual void BuildRootSignature() override;
    virtual void BuildShadersAndInputLayout() override;
    virtual void BuildShapeGeometry() override;
    virtual void BuildPSOs() override;

    virtual void LoadTextures() override;
    virtual void BuildFrameResources() override;
    virtual void BuildMaterials() override;
    virtual void BuildRenderItems() override;
    virtual void UploadTextures() override;

    virtual void InitGBuffer() override;
    void ResizeGBuffer();

    void RenderUI();

    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawScreenQuad(ID3D12GraphicsCommandList* cmdList);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignatureLight = nullptr;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayoutLight;

    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    // Render items divided by PSO.
    std::vector<RenderItem*> mOpaqueRitems;

    PassConstants mMainPassCB;
    PassConstants mReflectedPassCB;

    float mTheta = 1.3f * DirectX::XM_PI;
    float mPhi = 0.4f * DirectX::XM_PI;
    float mRadius = 2.5f;

    POINT mLastMousePos;

    Camera mCamera;

    float tilesCount = 1.0f;
};

// Imgui Variables
bool opened = true;
int tilesCountInt = 1;
int selectedObjectID = 1;
bool isAnimateMaterial = true;
bool isSolid = true;
bool deferredRenderDisplayInfo = false;
bool isParallaxMapping = false;

bool isPixelated = false;
int pixelationFactor = 16.f;

float Obj1posX = 2.f;
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

    if (isAnimateMaterial)
        AnimateMaterials(gt);

    UpdateObjectCBs(gt);
    UpdateMaterialCBs(gt);
    UpdateMainPassCB(gt);
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
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize), DirectX::Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearRenderTargetView(CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize), DirectX::Colors::LightSteelBlue, 0, nullptr);

    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] = {
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 2, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 3, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 4, mRtvDescriptorSize),
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), 5, mRtvDescriptorSize),
    };
    auto dsv = DepthStencilView();

    // Draw calls
    {
        ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap.Get() };
        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

        auto passCB = mCurrFrameResource->PassCB->Resource();
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
        D3D12_RESOURCE_BARRIER barriers[] = { backBuffer, albedo, position, normal, specular, depth};
        mCommandList->ResourceBarrier(6, barriers);

        mCommandList->OMSetRenderTargets(4, rtvs, false, &dsv);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        if (isSolid && !isPixelated) mCommandList->SetPipelineState(mPSOs["opaqueSolid"].Get());
        else if (isPixelated) mCommandList->SetPipelineState(mPSOs["opaquePixel"].Get());
        else mCommandList->SetPipelineState(mPSOs["opaqueWireframe"].Get());


        DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::OpaqueWireframe]);


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

        D3D12_CPU_DESCRIPTOR_HANDLE rtvs2 = CurrentBackBufferView();

        mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

        mCommandList->SetGraphicsRootSignature(mRootSignatureLight.Get());

        mCommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

        mCommandList->OMSetRenderTargets(1, &rtvs2, FALSE, nullptr);

        mCommandList->RSSetViewports(1, &viewports[4]);
        mCommandList->RSSetScissorRects(1, &rects[4]);

        mCommandList->SetPipelineState(mPSOs["deferredLighting"].Get());

        DrawScreenQuad(mCommandList.Get());
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

            DirectX::XMMATRIX texTransform = XMLoadFloat4x4(&mAllRitems[i]->TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

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
    mMainPassCB.Lights[3].Position = { lightPos1[0], lightPos1[1], lightPos1[2] };
    mMainPassCB.Lights[3].FalloffStart = 1.0f;
    mMainPassCB.Lights[3].FalloffEnd = 1.0f;
    mMainPassCB.Lights[3].Color = { col1[0], col1[1], col1[2], 1.0f };
    mMainPassCB.Lights[4].Position = { lightPos2[0], lightPos2[1], lightPos2[2] };
    mMainPassCB.Lights[4].FalloffStart = 1.0f;
    mMainPassCB.Lights[4].FalloffEnd = 1.0f;
    mMainPassCB.Lights[4].Color = { col2[0], col2[1], col2[2], 1.0f };
    mMainPassCB.Lights[5].Position = { lightPos3[0], lightPos3[1], lightPos3[2] };
    mMainPassCB.Lights[5].FalloffStart = 1.0f;
    mMainPassCB.Lights[5].FalloffEnd = 1.0f;
    mMainPassCB.Lights[5].Color = { col3[0], col3[1], col3[2], 1.0f };


    // Spot lights
    //mMainPassCB.Lights[6].FalloffStart = 1.0f;
    //mMainPassCB.Lights[6].FalloffEnd = 100.0f;
    //mMainPassCB.Lights[6].Color = { 1.0f, 0.0f, 1.0f, 1.0f };
    //mMainPassCB.Lights[6].SpotPower = 1000.0f;
    //XMStoreFloat3(&mMainPassCB.Lights[6].Direction, mCamera.GetLook());
    //XMStoreFloat3(&mMainPassCB.Lights[6].Position, mCamera.GetPosition());


    mMainPassCB.TessFactor = tessFactor;
    mMainPassCB.PixelationFactor = pixelationFactor;
    if (isParallaxMapping)
        mMainPassCB.ParallaxMapping = 1.0f;
    else mMainPassCB.ParallaxMapping = 0.0f;

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
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
    metalAnimateTex->Name = "MetalAnimateTex";
    metalAnimateTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalTex.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateTex->Filename.c_str(),
        metalAnimateTex->Resource, metalAnimateTex->UploadHeap);

    mTextures[metalAnimateTex->Name] = std::move(metalAnimateTex);

    auto metalAnimateNorm = std::make_unique<Texture>();
    metalAnimateNorm->Name = "MetalAnimateNorm";
    metalAnimateNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalNorm.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateNorm->Filename.c_str(),
        metalAnimateNorm->Resource, metalAnimateNorm->UploadHeap);

    mTextures[metalAnimateNorm->Name] = std::move(metalAnimateNorm);

    auto metalAnimateDisplacement = std::make_unique<Texture>();
    metalAnimateDisplacement->Name = "MetalAnimateDisplacement";
    metalAnimateDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalDisplacement.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateDisplacement->Filename.c_str(),
        metalAnimateDisplacement->Resource, metalAnimateDisplacement->UploadHeap);

    mTextures[metalAnimateDisplacement->Name] = std::move(metalAnimateDisplacement);

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
}

void Engine::BuildDescriptorHeaps()
{
    //
    // Create the SRV heap.
    //
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 14;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

    mSrvHeapAllocator.Create(md3dDevice.Get(), mSrvHeap.Get());
}

void Engine::UploadTextures()
{
    //
    // Fill out the heap with actual descriptors.
    //
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(1, mCbvSrvDescriptorSize);

    auto MetalTex = mTextures["MetalAnimateTex"]->Resource;
    auto MetalNorm = mTextures["MetalAnimateNorm"]->Resource;
    auto MetalDisplacement = mTextures["MetalAnimateDisplacement"]->Resource;


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


void Engine::BuildRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE texTable;
    texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 15, 0);
    CD3DX12_DESCRIPTOR_RANGE texTable2;
    texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[4];
    CD3DX12_ROOT_PARAMETER slotRootParameter2[2];

    // Perfomance TIP: Order from most frequent to least frequent.
    slotRootParameter[0].InitAsDescriptorTable(1, &texTable);
    slotRootParameter[1].InitAsConstantBufferView(0);
    slotRootParameter[2].InitAsConstantBufferView(1);
    slotRootParameter[3].InitAsConstantBufferView(2);

    slotRootParameter2[0].InitAsDescriptorTable(1, &texTable2);
    slotRootParameter2[1].InitAsConstantBufferView(0);

    auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
        (UINT)staticSamplers.size(), staticSamplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc2(2, slotRootParameter2, 
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
}


void Engine::BuildShadersAndInputLayout()
{
    mShaders["tessVS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "VS", "vs_5_0");
    mShaders["tessHS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "HS", "hs_5_0");
    mShaders["tessDS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "DS", "ds_5_0");
    mShaders["tessPS"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PS", "ps_5_0");
    mShaders["PSPixel"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\Tessellation.hlsl", nullptr, "PSPixel", "ps_5_0");

    mShaders["DeferredVSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "VSMain", "vs_5_0");
    mShaders["DeferredPSLighting"] = d3dUtil::CompileShader(L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Shaders\\DeferredLighting.hlsl", nullptr, "PSMain", "ps_5_0");

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
}

void Engine::BuildShapeGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box = geoGen.CreateBoxTiling(1.0f, 1.0f, 1.0f, 1, 1);
    GeometryGenerator::MeshData box2 = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box3 = geoGen.CreateBoxTiling(5.f, 5.f, 5.f, 1, 1);
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.2f, 7, 3, 0.f, 0.f, 0.f);

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
    totalVertexCount2 += box.Vertices.size();
    for (size_t i = 0; i < box3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box3.Vertices[i].TangentU;
    }
    totalVertexCount2 += box.Vertices.size();
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


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
    indices.insert(indices.end(), std::begin(box2.GetIndices16()), std::end(box2.GetIndices16()));
    indices.insert(indices.end(), std::begin(box3.GetIndices16()), std::end(box3.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

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
    geo->DrawArgs["pointLight1"] = pointLight1Submesh;
    geo->DrawArgs["pointLight2"] = pointLight2Submesh;
    geo->DrawArgs["pointLight3"] = pointLight3Submesh;

    mGeometries[geo->Name] = std::move(geo);
}


void Engine::BuildPSOs()
{
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
    stoneMat->DiffuseSrvHeapIndex = 4;
    stoneMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    stoneMat->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    stoneMat->Roughness = 0.5f;

    mMaterials["stoneMaterial"] = std::move(stoneMat);

    auto testMat = std::make_unique<Material>();
    testMat->Name = "testMaterial";
    testMat->MatCBIndex = 3;
    testMat->DiffuseSrvHeapIndex = 7;
    testMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    testMat->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    testMat->Roughness = 0.5f;

    mMaterials["testMaterial"] = std::move(testMat);
}

void Engine::BuildRenderItems()
{
    auto boxRitem = std::make_unique<RenderItem>();
    boxRitem->ObjCBIndex = 0;
    boxRitem->World = MathHelper::Identity4x4();
    boxRitem->Mat = mMaterials["stoneMaterial"].get();
    boxRitem->Geo = mGeometries["boxGeo"].get();
    boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    mAllRitems.push_back(std::move(boxRitem));

    auto boxTileRitem = std::make_unique<RenderItem>();
    boxTileRitem->ObjCBIndex = 1;
    XMStoreFloat4x4(&boxTileRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(2.0f, 2.0f, 0.0f));
    boxTileRitem->Mat = mMaterials["stoneMaterial"].get();
    boxTileRitem->Geo = mGeometries["boxGeo"].get();
    boxTileRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxTileRitem->IndexCount = boxTileRitem->Geo->DrawArgs["box2"].IndexCount;
    boxTileRitem->StartIndexLocation = boxTileRitem->Geo->DrawArgs["box2"].StartIndexLocation;
    boxTileRitem->BaseVertexLocation = boxTileRitem->Geo->DrawArgs["box2"].BaseVertexLocation;
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
    mAllRitems.push_back(std::move(stoneTesselationRitem));

    //auto objectModelRitem = std::make_unique<RenderItem>();
    //objectModelRitem->ObjCBIndex = 3;
    //XMStoreFloat4x4(&objectModelRitem->World, DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) * DirectX::XMMatrixTranslation(-8.0f, 0.0f, 0.0f));
    //objectModelRitem->TexTransform = MathHelper::Identity4x4();
    //objectModelRitem->Mat = mMaterials["testMaterial"].get();
    //objectModelRitem->Geo = mGeometries["boxGeo"].get();
    //objectModelRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    //objectModelRitem->IndexCount = objectModelRitem->Geo->DrawArgs["objectModel"].IndexCount;
    //objectModelRitem->StartIndexLocation = objectModelRitem->Geo->DrawArgs["objectModel"].StartIndexLocation;
    //objectModelRitem->BaseVertexLocation = objectModelRitem->Geo->DrawArgs["objectModel"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(objectModelRitem));

    //auto pointLight1Ritem = std::make_unique<RenderItem>();
    //pointLight1Ritem->ObjCBIndex = 3;
    //pointLight1Ritem->World = MathHelper::Identity4x4();
    //pointLight1Ritem->TexTransform = MathHelper::Identity4x4();
    //pointLight1Ritem->Mat = mMaterials["testMaterial"].get();
    //pointLight1Ritem->Geo = mGeometries["boxGeo"].get();
    //pointLight1Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    //pointLight1Ritem->IndexCount = pointLight1Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    //pointLight1Ritem->StartIndexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    //pointLight1Ritem->BaseVertexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(pointLight1Ritem));

    //auto pointLight2Ritem = std::make_unique<RenderItem>();
    //pointLight2Ritem->ObjCBIndex = 4;
    //pointLight2Ritem->World = MathHelper::Identity4x4();
    //pointLight2Ritem->TexTransform = MathHelper::Identity4x4();
    //pointLight2Ritem->Mat = mMaterials["testMaterial"].get();
    //pointLight2Ritem->Geo = mGeometries["boxGeo"].get();
    //pointLight2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    //pointLight2Ritem->IndexCount = pointLight2Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    //pointLight2Ritem->StartIndexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    //pointLight2Ritem->BaseVertexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(pointLight2Ritem));

    //auto pointLight3Ritem = std::make_unique<RenderItem>();
    //pointLight3Ritem->ObjCBIndex = 5;
    //pointLight3Ritem->World = MathHelper::Identity4x4();
    //pointLight3Ritem->TexTransform = MathHelper::Identity4x4();
    //pointLight3Ritem->Mat = mMaterials["testMaterial"].get();
    //pointLight3Ritem->Geo = mGeometries["boxGeo"].get();
    //pointLight3Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    //pointLight3Ritem->IndexCount = pointLight3Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    //pointLight3Ritem->StartIndexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    //pointLight3Ritem->BaseVertexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(pointLight3Ritem));

    // All the render items are opaque.
    for (int i = 0; i < mAllRitems.size(); ++i)
    {
        mRitemLayer[(int)RenderLayer::Opaque].push_back(mAllRitems[i].get());
        mRitemLayer[(int)RenderLayer::OpaqueWireframe].push_back(mAllRitems[i].get());
    }
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

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = mDepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

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
    hDescriptor.Offset(10, mCbvSrvDescriptorSize);

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

            cmdList->SetGraphicsRootDescriptorTable(0, tex);
            cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
            cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

            cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
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
    srvs.Offset(10, mCbvSrvDescriptorSize);

    cmdList->SetGraphicsRootDescriptorTable(0, srvs);

    cmdList->DrawInstanced(4, 1, 0, 0);
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

void Engine::RenderUI()
{
    ImVec2 size = ImVec2(WINDOW_WIDTH / 5, WINDOW_HEIGHT / 4 * 3);
    ImVec2 pos = ImVec2(WINDOW_WIDTH - size.x, 0);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowBgAlpha(0.5f);
    if (ImGui::Begin("Configuration", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("");
        ImGui::Checkbox("Deferred Render Info", &deferredRenderDisplayInfo);
        ImGui::Checkbox("Parallax Mapping", &isParallaxMapping);
        ImGui::Checkbox("Solid Mode", &isSolid);
        ImGui::SliderFloat("Tesselation Factor", &tessFactor, 1.f, 64.f);
        ImGui::Checkbox("Pixelation Shader", &isPixelated);
        ImGui::SliderInt("Pixelated Factor", &pixelationFactor, 16.f, 128.f);
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
    } ImGui::End();

    ImVec2 lightPanelSize = ImVec2(350.f, 350.f);
    ImGui::SetNextWindowSize(lightPanelSize);
    if (ImGui::Begin("Light Config", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
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
        ImGui::ColorPicker3("Point light 1 color", col1, ImGuiColorEditFlags_NoAlpha);

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
        ImGui::ColorPicker3("Point light 2 color", col2, ImGuiColorEditFlags_NoAlpha);

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
        ImGui::ColorPicker3("Point light 3 color", col3, ImGuiColorEditFlags_NoAlpha);
    } ImGui::End();


    if (deferredRenderDisplayInfo)
    {
        ImVec2 size2 = ImVec2(WINDOW_WIDTH / 5, 800);
        ImVec2 pos2 = ImVec2(0, 0);

        ImGui::SetNextWindowPos(pos2);
        ImGui::SetNextWindowSize(size2);
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Deferred Render Info", &opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvHeap->GetGPUDescriptorHandleForHeapStart());
            tex.Offset(10, mCbvSrvDescriptorSize);
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

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());
}
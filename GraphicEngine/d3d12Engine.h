//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <d3d12.h>
#include "d3dUtil.h"
#include "GameTimer.h"
#include "GBuffer.h"

// Link necessary d3d12 libraries.
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "GeometryManager.h"
#include "InputLayoutShaderManager.h"
#include "RootSignatureManager.h"
#include "PSOManager.h"
#include "DescriptorHeapManager.h"

// Simple free list based allocator

class D3D12Engine
{
protected:

    D3D12Engine(HINSTANCE hInstance);
    D3D12Engine(const D3D12Engine& rhs) = delete;
    D3D12Engine& operator=(const D3D12Engine& rhs) = delete;
    virtual ~D3D12Engine();

public:

    static D3D12Engine* GetApp();

    HINSTANCE AppInst()const;
    HWND      MainWnd()const;
    float     AspectRatio()const;

    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

    virtual int Run() = 0;

    virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    virtual void OnResize();
    virtual void Update(const GameTimer& gt) = 0;
    virtual void Draw(const GameTimer& gt) = 0;

    // Convenience overrides for handling mouse input.
    virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
    virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
    virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

    
    virtual void InitGBuffer() {}
    virtual void CreateScene3RTV() {}
    virtual void CreateScene13RTV() {}
    virtual void CreateScene15RTV() {}

    virtual void InitInstanceBuffer() {}
    virtual void InitInstanceBufferRMDemo() {}
    virtual void InitInstanceBufferMoreLight() {}

    virtual void InitParticleSystem() {}
    virtual void InitMarchingCubesSystem() {}
    virtual void BuildPostProcessingResources() {}
    virtual void CreateNoiseTexture() {}

    virtual void BuildFrameResources() {}
    virtual void BuildBillboardSpritesGeometry() {}

protected:

    bool InitMainWindow();
    bool InitDirect3D();
    void InitImgui();
    void CreateCommandObjects();
    void CreateSwapChain();

    void FlushCommandQueue();

    ID3D12Resource* CurrentBackBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE OtherBackBufferView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilShadowsView()const;

    ID3D12Resource* Scene3RenderTargetBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE Scene3RenderTargetBufferView()const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE DepthStencilViewScene3()const;

    ID3D12Resource* Scene13RenderTargetBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE Scene13RenderTargetBufferView()const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE DepthStencilViewScene13()const;

    void CalculateFrameStats();

    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected:

    static D3D12Engine* mApp;

    HINSTANCE mhAppInst = nullptr; // application instance handle
    HWND      mhMainWnd = nullptr; // main window handle
    bool      mAppPaused = false;  // is the application paused?
    bool      mMinimized = false;  // is the application minimized?
    bool      mMaximized = false;  // is the application maximized?
    bool      mResizing = false;   // are the resize bars being dragged?
    bool      mFullscreenState = false;// fullscreen enabled

    // Set true to use 4X MSAA (§4.1.8).  The default is false.
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

    // Used to keep track of the “delta-time” and game time (§4.4).
    GameTimer mTimer;

    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandSignature> mCommandSignature;

    static const int SwapChainBufferCount = 2;
    int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargetBufferScene3;
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBufferScene3;
    
    Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargetBufferScene13;
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBufferScene13;

    GBuffer gBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
    D3D12_VERTEX_BUFFER_VIEW instanceVertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceUploadBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> instanceRMDemoBuffer;
    D3D12_VERTEX_BUFFER_VIEW instanceRMDemoVertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceRMDemoUploadBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> instanceMoreLightBuffer;
    D3D12_VERTEX_BUFFER_VIEW instanceMoreLightVertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceMoreLightUploadBuffer;


    D3D12_VERTEX_BUFFER_VIEW instanceOcTreeVertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> particleBuffers[2];
    Microsoft::WRL::ComPtr<ID3D12Resource> particleArgsBuffer;
    UINT currParticleReadBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> particle2Buffers[2];
    Microsoft::WRL::ComPtr<ID3D12Resource> particle2ArgsBuffer;
    UINT currParticle2ReadBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> particleSmokeBuffers[2];
    Microsoft::WRL::ComPtr<ID3D12Resource> particleSmokeArgsBuffer;
    UINT currParticleSmokeReadBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> particleRainBuffers[2];
    Microsoft::WRL::ComPtr<ID3D12Resource> particleRainArgsBuffer;
    UINT currParticleRainReadBuffer = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> postProcessingBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> noiseTexture;

    D3D12_VIEWPORT mScreenViewportFull;
    D3D12_VIEWPORT mScreenViewport;
    D3D12_VIEWPORT mScreenViewport2;
    D3D12_VIEWPORT mScreenViewport3;
    D3D12_VIEWPORT mScreenViewport4;

    D3D12_RECT mScissorRectFull;
    D3D12_RECT mScissorRect;
    D3D12_RECT mScissorRect2;
    D3D12_RECT mScissorRect3;
    D3D12_RECT mScissorRect4;

    // Derived class should set these in derived constructor to customize starting values.
    std::wstring mMainWndCaption = L"D3D12Engine";
    D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    int mClientWidth = 1280;
    int mClientHeight = 800;
};


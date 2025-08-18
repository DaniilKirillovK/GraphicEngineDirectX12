#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "Instancing.h"

struct EmitterConstants
{
    DirectX::XMFLOAT3 Position;
    float DeltaTime;
    DirectX::XMFLOAT3 GravityForce;
    UINT SystemID;
    DirectX::XMFLOAT4 StartColor;
    DirectX::XMFLOAT4 EndColor;
    float StartSize;
    float EndSize;
    UINT MaxParticles;
    UINT EmitterIsActive;
    float TotalTime;
};

struct PostProcessingConstants
{
    float gGammaRatio;
    float gTextureSize;
    DirectX::XMFLOAT2 CADistortion;
    DirectX::XMFLOAT2 CADirection;
    bool GBIsHorizontal;
    float CAIntensity;
    float CAPadding;
    DirectX::XMFLOAT2 VCenter;
    float VIntensity;
    float VSmoothness;
    float VRoundness;
    float NIntensity;
    float NSize;
};

struct NoiseComputeConstants
{
    float TotalTime;
};

struct MoreSamplersConstants
{
    UINT Flitering;
    UINT AddressMode;
};

struct LODConstants
{
    UINT LevelOfDetail;
};

struct TessConstants
{
    float TessFactor = 1.f;
    UINT bIsBackCulling = 0;
    UINT DisplacementAdaptiveTess = 0;
    UINT DistantAdaptiveTess = 0;

    Decal Decals[MaxDecals];
};

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    float isTessellationNeeded = 1.0f;
    float scale = 1.0f;
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    DirectX::XMFLOAT4 FogColor = { 0.7f, 0.7f, 0.7f, 1.0f };
    float gFogStart = 5.0f;
    float gFogRange = 150.0f;
    DirectX::XMFLOAT2 cbPerObjectPad2;

    float TessFactor = 1.f;
    float PixelationFactor = 16.f;
    float ParallaxMapping = 0.f;
    float displacementLevel = 1.0f;

    float isNegative = 0.0f;
    DirectX::XMFLOAT3 cbPad;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light Lights[MaxLights];
};

struct Vertex
{
    Vertex() = default;
    Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) :
        Pos(x, y, z),
        Normal(nx, ny, nz),
        TexC(u, v) {
    }

    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT3 TangentU;
};

// Stores the resources needed for the CPU to build the command lists
// for a frame.  
struct FrameResource
{
public:

    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

    // We cannot reset the allocator until the GPU is done processing the commands.
    // So each frame needs their own allocator.
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

    // We cannot update a cbuffer until the GPU is done processing the commands
    // that reference it.  So each frame needs their own cbuffers.
   // std::unique_ptr<UploadBuffer<FrameConstants>> FrameCB = nullptr;
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<PassConstants>> PassCBScene3Camera2 = nullptr;
    std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    std::unique_ptr<UploadBuffer<EmitterConstants>> EmitterCB = nullptr;
    std::unique_ptr<UploadBuffer<EmitterConstants>> Emitter2CB = nullptr;
    std::unique_ptr<UploadBuffer<InstanceData>> InstancingCB = nullptr;
    std::unique_ptr<UploadBuffer<PostProcessingConstants>> PostProcessingCB = nullptr;
    std::unique_ptr<UploadBuffer<NoiseComputeConstants>> NoiseCB = nullptr;
    std::unique_ptr<UploadBuffer<MoreSamplersConstants>> SamplersCB = nullptr;
    std::unique_ptr<UploadBuffer<LODConstants>> LODCB = nullptr;
    std::unique_ptr<UploadBuffer<TessConstants>> TessCB = nullptr;

    // Fence value to mark commands up to this fence point.  This lets us
    // check if these frame resources are still in use by the GPU.
    UINT64 Fence = 0;
};

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

    DirectX::BoundingBox Bounds;
    std::vector<InstanceData> Instances;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT InstanceCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

    // DrawInstanced parameters
    UINT VertexCount = 0;
};
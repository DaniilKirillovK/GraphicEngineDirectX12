#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    //  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    PassCBScene3Camera2 = std::make_unique<UploadBuffer<PassConstants>>(device, 1, true);
    PassCBScene13Camera2 = std::make_unique<UploadBuffer<PassConstants>>(device, 1, true);
    PassCBShadows = std::make_unique<UploadBuffer<PassConstantsShadows>>(device, 1, true);
    PassCBShadowsCascaded = std::make_unique<UploadBuffer<PassConstantsShadows>>(device, 4, true);
    ShadowPassCB = std::make_unique<UploadBuffer<PassConstantsShadows>>(device, 1, true);
    ShadowPassCBCascaded = std::make_unique<UploadBuffer<PassConstantsShadows>>(device, 4, true);
    ShadowPassCBParticles = std::make_unique<UploadBuffer<PassConstantsShadows>>(device, 1, true);
    MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
    LightObjectCB = std::make_unique<UploadBuffer<LightObjectConstants>>(device, 5, true);
    EmitterCB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    Emitter2CB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    Emitter3CB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    Emitter4CB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    InstancingCB = std::make_unique<UploadBuffer<InstanceData>>(device, 900, false);
    InstancingScene13CB = std::make_unique<UploadBuffer<InstanceDataGameObject>>(device, 150, false);
    PostProcessingCB = std::make_unique<UploadBuffer<PostProcessingConstants>>(device, 1, true);
    NoiseCB = std::make_unique<UploadBuffer<NoiseComputeConstants>>(device, 1, true);
    SamplersCB = std::make_unique<UploadBuffer<MoreSamplersConstants>>(device, 1, true);
    LODCB = std::make_unique<UploadBuffer<LODConstants>>(device, 2, true);
    TessCB = std::make_unique<UploadBuffer<TessConstants>>(device, 1, true);
    HeightMapCB = std::make_unique<UploadBuffer<HeightMapConstants>>(device, 1, true);
    TerrainCB = std::make_unique<UploadBuffer<TerrainConstants>>(device, 1, true);
    TAAPassCB = std::make_unique<UploadBuffer<TAAPassConstants>>(device, 1, true);
    TAAObjectsCB = std::make_unique<UploadBuffer<TAAObjectConstants>>(device, objectCount, true);
    AtmosphereCB = std::make_unique<UploadBuffer<AtmosphereConstants>>(device, 1, true);
    PaintCB = std::make_unique<UploadBuffer<PaintConstants>>(device, 1, true);
    HorizontalBlurCB = std::make_unique<UploadBuffer<BlurConstants>>(device, 1, true);
    VerticalBlurCB = std::make_unique<UploadBuffer<BlurConstants>>(device, 1, true);
    BloomCombineCB = std::make_unique<UploadBuffer<CombineConstants>>(device, 1, true);
    ExtractBrightCB = std::make_unique<UploadBuffer<ExtractBrightConstants>>(device, 1, true);
}

FrameResource::~FrameResource()
{

}
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

    //  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    PassCBScene3Camera2 = std::make_unique<UploadBuffer<PassConstants>>(device, 1, true);
    MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(device, materialCount, true);
    ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
    EmitterCB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    Emitter2CB = std::make_unique<UploadBuffer<EmitterConstants>>(device, 1, true);
    InstancingCB = std::make_unique<UploadBuffer<InstanceData>>(device, 900, false);
    PostProcessingCB = std::make_unique<UploadBuffer<PostProcessingConstants>>(device, 1, true);
    NoiseCB = std::make_unique<UploadBuffer<NoiseComputeConstants>>(device, 1, true);
    SamplersCB = std::make_unique<UploadBuffer<MoreSamplersConstants>>(device, 1, true);
    LODCB = std::make_unique<UploadBuffer<LODConstants>>(device, 1, true);
    TessCB = std::make_unique<UploadBuffer<TessConstants>>(device, 1, true);
}

FrameResource::~FrameResource()
{

}
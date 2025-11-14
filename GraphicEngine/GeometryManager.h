#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "D3D12Engine.h"
#include "FrameResource.h"
#include <vector>
#include <string>

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

class GeometryManager
{
public:
    static std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    static std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    static std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    static std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    static std::vector<RenderItem*> mRitemLayer[];

    static void BuildMaterials();
    static void BuildRenderItems();
    static void LoadTextures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);
};


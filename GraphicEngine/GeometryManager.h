#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "D3D12Engine.h"
#include "FrameResource.h"
#include <vector>
#include <string>
#include "ModelLoader.h"
#include "QuadTree.h"

class Model;

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
    Scene18 = 28,
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

    static void BuildSkyboxGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildShadowMaps(Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildShapeGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene3Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene3LODGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene4Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene5Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene6Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene7Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene9Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene9RMDemoGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildModelsGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene10DebugGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene10MoreLightGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene12Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene13Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene13InstanceBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene13OctreeGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene14Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildScene15Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildQuadTreeTerrain(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);

    static void BuildSponzaGeometryAndTextures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
        Microsoft::WRL::ComPtr<ID3D12Device> device);


    static Model Sponza;
    static Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24];
    static Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24];
    static TerrainQuadtree m_terrainQuadTree;

    static Microsoft::WRL::ComPtr<ID3D12Resource> instanceOcTreeBuffer;
    static Microsoft::WRL::ComPtr<ID3D12Resource> instanceOcTreeUploadBuffer;
};


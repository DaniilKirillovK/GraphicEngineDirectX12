#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <d3d12.h>
#include "FrameResource.h"
#include <queue>
#include "GeometryGenerator.h"
#include <cmath>

struct StaticMesh
{
    int materialIndex;
    DirectX::BoundingBox boundingBox;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
};

struct QuadTreeTerrainDebugVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT3 TangentU;
    DirectX::XMFLOAT4 Color;
};


struct QuadTreeNode 
{
    DirectX::BoundingBox bounds;
    std::array<QuadTreeNode*, 4> children;
    uint32_t depth;
    bool isLeaf;
    StaticMesh mesh;
    StaticMesh debugMesh;

    QuadTreeNode() 
    {
        children.fill(nullptr);
        isLeaf = true;
        depth = 0;
    }
};


class TerrainQuadtree 
{
public:
    TerrainQuadtree();
    ~TerrainQuadtree();

    bool Initialize(ID3D12Device* device, uint32_t maxDepth,
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void BuildTree(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void Update(const DirectX::XMMATRIX& viewProj, const DirectX::BoundingFrustum& cameraFrustum);

    std::vector<QuadTreeNode*> m_NodesToRender;
    std::vector<QuadTreeNode*> m_DebugNodesToRender;

private:
    void BuildNode(QuadTreeNode* node, const DirectX::BoundingBox& bounds, uint32_t depth,
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void SubdivideNode(QuadTreeNode* node);
    void UpdateNode(QuadTreeNode* node, const DirectX::BoundingFrustum& cameraFrustum);

    ID3D12Device* m_device;
    QuadTreeNode* m_root;
    uint32_t m_maxDepth;

    std::vector<float> distances = { 1024.f, 512.f, 256.f, 128.f, 64.f, 32.f, 0.f };
};


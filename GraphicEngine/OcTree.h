#pragma once
#include <DirectXCollision.h>
#include <vector>
#include <memory>
#include <cstdint>
#include <d3d12.h>
#include "FrameResource.h"

struct GameObject
{
    DirectX::BoundingBox boundingBox;
    int objectID;
};

struct DebugVertexOcTree 
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT4 Color;
};


struct OctreeNode 
{
    DirectX::BoundingBox Bounds;
    std::vector<GameObject*> Objects;
    std::unique_ptr<OctreeNode> Children[8];
    bool IsLeaf = true;
    uint32_t Depth = 0;

    uint32_t ObjectCount = 0;

    OctreeNode(const DirectX::BoundingBox& nodeBounds, uint32_t depth = 0);
    ~OctreeNode() = default;

    bool Contains(const DirectX::BoundingBox& objectBounds) const;
    void CreateChildren();
    void Clear();
};


class Octree 
{
public:
    Octree(ID3D12Device* device, const DirectX::BoundingBox& worldBounds);
    Octree() {}
    ~Octree() {}

    void Build(const std::vector<GameObject*>& objects);
    void Insert(GameObject* object);

    OctreeNode* GetRoot();

    void CollectDebugNodes(OctreeNode* node, std::vector<DebugVertexOcTree>& vertices,
        std::vector<uint16_t>& indices) const;

    void OctreeCulling(OctreeNode* node,
        const DirectX::BoundingFrustum& frustum,
        std::vector<GameObject*>& visibleObjects);

private:
    std::unique_ptr<OctreeNode> m_Root;
    Microsoft::WRL::ComPtr<ID3D12Device> mDevice;

    uint32_t m_MaxDepth = 5;
    uint32_t m_MinObjectsPerNode = 4;
    uint32_t m_NodeCount = 0;

    bool m_DrawDebug = false;

    void InsertRecursive(OctreeNode* node, GameObject* object);
};
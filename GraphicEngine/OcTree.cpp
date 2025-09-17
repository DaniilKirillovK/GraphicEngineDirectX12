#include "Octree.h"

OctreeNode::OctreeNode(const DirectX::BoundingBox& nodeBounds, uint32_t depth)
    : Bounds(nodeBounds), Depth(depth) {}

bool OctreeNode::Contains(const DirectX::BoundingBox& objectBounds) const 
{
    return Bounds.Contains(objectBounds) != DirectX::DISJOINT;
}

void OctreeNode::CreateChildren() 
{
    if (!IsLeaf || Depth >= 8) return;

    DirectX::XMFLOAT3 center = Bounds.Center;
    DirectX::XMFLOAT3 extents = Bounds.Extents;
    DirectX::XMFLOAT3 childExtents = { extents.x * 0.5f, extents.y * 0.5f, extents.z * 0.5f };

    for (uint32_t i = 0; i < 8; ++i) 
    {
        DirectX::XMFLOAT3 childCenter = center;
        childCenter.x += (i & 1) ? childExtents.x : -childExtents.x;
        childCenter.y += (i & 2) ? childExtents.y : -childExtents.y;
        childCenter.z += (i & 4) ? childExtents.z : -childExtents.z;

        DirectX::BoundingBox childBounds;
        childBounds.Center = childCenter;
        childBounds.Extents = childExtents;

        Children[i] = std::make_unique<OctreeNode>(childBounds, Depth + 1);
    }

    IsLeaf = false;
}

void OctreeNode::Clear() 
{
    Objects.clear();
    ObjectCount = 0;
    for (auto& child : Children) 
    {
        if (child) 
        {
            child->Clear();
        }
    }
}

Octree::Octree(ID3D12Device* device, const DirectX::BoundingBox& worldBounds)
    : mDevice(device) 
{
    m_Root = std::make_unique<OctreeNode>(worldBounds, 0);
}

void Octree::Build(const std::vector<GameObject*>& objects) 
{
    m_Root->Clear();
    m_NodeCount = 1;

    for (auto* obj : objects) 
    {
        Insert(obj);
    }
}

void Octree::Insert(GameObject* object) 
{
    InsertRecursive(m_Root.get(), object);
}

OctreeNode* Octree::GetRoot()
{
    return m_Root.get();
}

void Octree::InsertRecursive(OctreeNode* node, GameObject* object) 
{
    const auto& objectBounds = object->boundingBox;

    if (!node->Contains(objectBounds)) 
    {
        return;
    }

    if (node->IsLeaf) 
    {
        node->Objects.push_back(object);
        node->ObjectCount++;

        if (node->Objects.size() > m_MinObjectsPerNode && node->Depth < m_MaxDepth) 
        {
            node->CreateChildren();
            m_NodeCount += 8;

            auto objectsToRedistribute = std::move(node->Objects);
            node->ObjectCount = 0;

            for (auto* obj : objectsToRedistribute) 
            {
                for (auto& child : node->Children) 
                {
                    InsertRecursive(child.get(), obj);
                }
            }
        }
    }
    else 
    {
        for (auto& child : node->Children) 
        {
            InsertRecursive(child.get(), object);
        }
    }
}

void Octree::CollectDebugNodes(OctreeNode* node,
    std::vector<DebugVertexOcTree>& vertices,
    std::vector<uint16_t>& indices) const 
{
    const auto& min = DirectX::XMFLOAT3
    (
        node->Bounds.Center.x - node->Bounds.Extents.x,
        node->Bounds.Center.y - node->Bounds.Extents.y,
        node->Bounds.Center.z - node->Bounds.Extents.z
    );

    const auto& max = DirectX::XMFLOAT3
    (
        node->Bounds.Center.x + node->Bounds.Extents.x,
        node->Bounds.Center.y + node->Bounds.Extents.y,
        node->Bounds.Center.z + node->Bounds.Extents.z
    );

    DirectX::XMFLOAT4 color;
    switch (node->Depth % 6) 
    {
    case 0: color = { 1.0f, 0.0f, 0.0f, 1.0f }; break;
    case 1: color = { 0.0f, 1.0f, 0.0f, 1.0f }; break;
    case 2: color = { 0.0f, 0.0f, 1.0f, 1.0f }; break;
    case 3: color = { 1.0f, 1.0f, 0.0f, 1.0f }; break;
    case 4: color = { 1.0f, 0.0f, 1.0f, 1.0f }; break;
    case 5: color = { 0.0f, 1.0f, 1.0f, 1.0f }; break;
    }

    uint32_t baseVertexIndex = static_cast<uint32_t>(vertices.size());

    DebugVertexOcTree v1;
    v1.Position = DirectX::XMFLOAT3(min.x, min.y, min.z);
    v1.Color = color;
    DebugVertexOcTree v2;
    v2.Position = DirectX::XMFLOAT3(max.x, min.y, min.z);
    v2.Color = color;
    DebugVertexOcTree v3;
    v3.Position = DirectX::XMFLOAT3(max.x, max.y, min.z);
    v3.Color = color;
    DebugVertexOcTree v4;
    v4.Position = DirectX::XMFLOAT3(min.x, max.y, min.z);
    v4.Color = color;
    DebugVertexOcTree v5;
    v5.Position = DirectX::XMFLOAT3(min.x, min.y, max.z);
    v5.Color = color;
    DebugVertexOcTree v6;
    v6.Position = DirectX::XMFLOAT3(max.x, min.y, max.z);
    v6.Color = color;
    DebugVertexOcTree v7;
    v7.Position = DirectX::XMFLOAT3(max.x, max.y, max.z);
    v7.Color = color;
    DebugVertexOcTree v8;
    v8.Position = DirectX::XMFLOAT3(min.x, max.y, max.z);
    v8.Color = color;

    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v4);
    vertices.push_back(v5);
    vertices.push_back(v6);
    vertices.push_back(v7);
    vertices.push_back(v8);

    const uint32_t cubeIndices[36] = 
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 3, 7, 4, 0, 3,
        1, 5, 6, 1, 6, 2,
        0, 5, 1, 0, 4, 5,
        3, 2, 6, 3, 6, 7
    };

    for (uint16_t i = 0; i < 36; ++i) 
    {
        indices.push_back(baseVertexIndex + cubeIndices[i]);
    }

    if (!node->IsLeaf) 
    {
        for (auto& child : node->Children) 
        {
            CollectDebugNodes(child.get(), vertices, indices);
        }
    }
}

void Octree::OctreeCulling(OctreeNode* node, const DirectX::BoundingFrustum& frustum, std::vector<GameObject*>& visibleObjects)
{
    if (!node) return;

    auto containment = frustum.Contains(node->Bounds);

    if (containment == DirectX::DISJOINT) 
    {
        return;
    }

    if (containment != DirectX::DISJOINT && node->IsLeaf)
    {
        for (int i = 0; i < node->Objects.size(); ++i)
        {
            visibleObjects.push_back(node->Objects[i]);
        }
    }

    if (!node->IsLeaf) 
    {
        for (int i = 0; i < 8; ++i) 
        {
            OctreeCulling(node->Children[i].get(), frustum, visibleObjects);
        }
    }
}

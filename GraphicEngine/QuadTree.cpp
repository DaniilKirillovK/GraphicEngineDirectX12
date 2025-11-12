#include "QuadTree.h"


TerrainQuadtree::TerrainQuadtree()
    : m_device(nullptr), m_root(nullptr), m_maxDepth(0) {}


TerrainQuadtree::~TerrainQuadtree() 
{
}

bool TerrainQuadtree::Initialize(ID3D12Device* device, uint32_t maxDepth,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    m_device = device;
    m_maxDepth = maxDepth;

    BuildTree(geometries, commandList);
    return true;
}

void TerrainQuadtree::BuildTree(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    DirectX::BoundingBox terrainBounds;
    terrainBounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    terrainBounds.Extents = DirectX::XMFLOAT3(256.f, 256.f, 256.f);

    m_root = new QuadTreeNode();
    BuildNode(m_root, terrainBounds, 0, geometries, commandList);
}


void TerrainQuadtree::BuildNode(QuadTreeNode* node,
    const DirectX::BoundingBox& bounds,
    uint32_t depth, std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    node->bounds = bounds;
    node->depth = depth;

    // Create Node mesh
    StaticMesh resultMesh;
    StaticMesh debugMesh;
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData Mesh = geoGen.CreateGrid(bounds.Extents.x * 2, bounds.Extents.z * 2, 10, 10);

    UINT totalIndexCount = (UINT)Mesh.Indices32.size();
    UINT totalVertexCount = Mesh.Vertices.size();
    std::vector<QuadTreeTerrainDebugVertex> debugVertices(totalVertexCount);
    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;
    for (size_t j = 0; j < Mesh.Vertices.size(); ++j)
    {
        DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
        DirectX::XMVECTOR v2 = XMLoadFloat3(&Mesh.Vertices[j].Position);
        DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
        XMStoreFloat3(&vertices[j].Pos, pos);

        DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
            (vertices[j].Pos.x + 256.0f) / 512.0f,
            (vertices[j].Pos.z + 256.0f) / 512.0f
        );

        vertices[j].Normal = Mesh.Vertices[j].Normal;
        vertices[j].TexC = newTexC;
        vertices[j].TangentU = Mesh.Vertices[j].TangentU;

        debugVertices[j].Pos = vertices[j].Pos;
        debugVertices[j].Normal = vertices[j].Normal;
        debugVertices[j].TexC = vertices[j].TexC;
        debugVertices[j].TangentU = vertices[j].TangentU;
        debugVertices[j].Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);
    }
    indices.insert(indices.end(), std::begin(Mesh.GetIndices16()), std::end(Mesh.GetIndices16()));
    // Skirts
    {
        // First Side
        totalVertexCount = vertices.size();
        // Up
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x + (bounds.Extents.x * j * 2) / 9.0f, 
                0.0f,
                -bounds.Extents.z);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        // Down
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x + (bounds.Extents.x * j * 2) / 9.0f,
                0.0f,
                -bounds.Extents.z);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        UINT startIndex = totalVertexCount;
        for (uint32_t i = 0; i < 9; ++i) 
        {
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + 10 + i + 1);
            indices.push_back(startIndex + 10 + i);

            indices.push_back(startIndex + i);
            indices.push_back(startIndex + i + 1);
            indices.push_back(startIndex + i + 10 + 1);
        }

        // Second Side
        totalVertexCount = vertices.size();
        // Up
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x + (bounds.Extents.x * j * 2) / 9.0f,
                0.0f,
                bounds.Extents.z);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        // Down
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x + (bounds.Extents.x * j * 2) / 9.0f,
                0.0f,
                bounds.Extents.z);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        startIndex = totalVertexCount;
        for (uint32_t i = 0; i < 9; ++i)
        {
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + 10 + i);
            indices.push_back(startIndex + 10 + i + 1);

            indices.push_back(startIndex + i + 1);
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + i + 10 + 1);
        }

        // Third Side
        totalVertexCount = vertices.size();
        // Up
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x,
                0.0f,
                -bounds.Extents.z + (bounds.Extents.z * j * 2) / 9.0f);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        // Down
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;

            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                -bounds.Extents.x,
                0.0f,
                -bounds.Extents.z + (bounds.Extents.z * j * 2) / 9.0f);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);

            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );

            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);

            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        startIndex = totalVertexCount;
        for (uint32_t i = 0; i < 9; ++i)
        {
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + 10 + i + 1);
            indices.push_back(startIndex + 10 + i);

            indices.push_back(startIndex + i);
            indices.push_back(startIndex + i + 1);
            indices.push_back(startIndex + i + 10 + 1);
        }

        // Fourth Side
        totalVertexCount = vertices.size();
        // Up
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;
        
            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                bounds.Extents.x,
                0.0f,
                -bounds.Extents.z + (bounds.Extents.z * j * 2) / 9.0f);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);
        
            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );
        
            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        
            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);
        
            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        // Down
        for (size_t j = 0; j < 10; ++j)
        {
            Vertex skirtVertex;
            QuadTreeTerrainDebugVertex debugSkirtVertex;
        
            DirectX::XMVECTOR v1 = XMLoadFloat3(&bounds.Center);
            DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(
                bounds.Extents.x,
                0.0f,
                -bounds.Extents.z + (bounds.Extents.z * j * 2) / 9.0f);
            DirectX::XMVECTOR v2 = XMLoadFloat3(&Position);
            DirectX::XMVECTOR pos = DirectX::XMVectorAdd(v1, v2);
            DirectX::XMFLOAT3 posDif = DirectX::XMFLOAT3(0.0f, -10.0f, 0.0f);
            DirectX::XMVECTOR vPosDif = XMLoadFloat3(&posDif);
            pos = DirectX::XMVectorAdd(pos, vPosDif);
            XMStoreFloat3(&skirtVertex.Pos, pos);
        
            DirectX::XMFLOAT2 newTexC = DirectX::XMFLOAT2(
                (skirtVertex.Pos.x + 256.f) / 512.f,
                (skirtVertex.Pos.z + 256.f) / 512.f
            );
        
            skirtVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            skirtVertex.TexC = newTexC;
            skirtVertex.TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
        
            debugSkirtVertex.Pos = skirtVertex.Pos;
            debugSkirtVertex.Normal = skirtVertex.Normal;
            debugSkirtVertex.TexC = skirtVertex.TexC;
            debugSkirtVertex.TangentU = skirtVertex.TangentU;
            debugSkirtVertex.Color = DirectX::XMFLOAT4(1.0f, 1.0f - (float)depth / 6.0f, 1.0f - (float)depth / 6.0f, 1.0f);
        
            vertices.push_back(skirtVertex);
            debugVertices.push_back(debugSkirtVertex);
        }
        startIndex = totalVertexCount;
        for (uint32_t i = 0; i < 9; ++i)
        {
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + 10 + i);
            indices.push_back(startIndex + 10 + i + 1);
        
            indices.push_back(startIndex + i + 1);
            indices.push_back(startIndex + i);
            indices.push_back(startIndex + i + 10 + 1);
        }

        totalIndexCount = indices.size();
    }

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT vbByteSizeDebug = (UINT)debugVertices.size() * sizeof(QuadTreeTerrainDebugVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    auto geoDebug = std::make_unique<MeshGeometry>();
    geo->Name = "QuadTreeTerrain" + std::to_string((int)bounds.Center.x * 512 + (int)bounds.Center.z);
    geoDebug->Name = "QuadTreeTerrainDebug" + std::to_string((int)bounds.Center.x * 512 + (int)bounds.Center.z);

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device,
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device,
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    resultMesh.vertices = vertices;
    resultMesh.indices = indices;
    resultMesh.vertexBufferView = geo->VertexBufferView();
    resultMesh.indexBufferView = geo->IndexBufferView();
    resultMesh.indexBuffer = geo->IndexBufferGPU.Get();
    resultMesh.vertexBuffer = geo->VertexBufferGPU.Get();

    geometries[geo->Name] = std::move(geo);

    node->mesh = resultMesh;

    // Create Debug Node mesh
    ThrowIfFailed(D3DCreateBlob(vbByteSizeDebug, &geoDebug->VertexBufferCPU));
    CopyMemory(geoDebug->VertexBufferCPU->GetBufferPointer(), debugVertices.data(), vbByteSizeDebug);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geoDebug->IndexBufferCPU));
    CopyMemory(geoDebug->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geoDebug->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device,
        commandList.Get(), debugVertices.data(), vbByteSizeDebug, geoDebug->VertexBufferUploader);

    geoDebug->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device,
        commandList.Get(), indices.data(), ibByteSize, geoDebug->IndexBufferUploader);

    geoDebug->VertexByteStride = sizeof(QuadTreeTerrainDebugVertex);
    geoDebug->VertexBufferByteSize = vbByteSizeDebug;
    geoDebug->IndexFormat = DXGI_FORMAT_R16_UINT;
    geoDebug->IndexBufferByteSize = ibByteSize;

    debugMesh.indices = indices;
    debugMesh.vertexBufferView = geoDebug->VertexBufferView();
    debugMesh.indexBufferView = geoDebug->IndexBufferView();
    debugMesh.indexBuffer = geoDebug->IndexBufferGPU.Get();
    debugMesh.vertexBuffer = geoDebug->VertexBufferGPU.Get();

    geometries[geoDebug->Name] = std::move(geoDebug);

    node->mesh = resultMesh;
    node->debugMesh = debugMesh;


    bool shouldSubdivide = (depth < m_maxDepth);

    if (shouldSubdivide) 
    {
        SubdivideNode(node);
        node->isLeaf = false;

        for (int i = 0; i < 4; ++i) 
        {
            BuildNode(node->children[i], node->children[i]->bounds, depth + 1, geometries, commandList);
        }
    }
    else 
    {
        node->isLeaf = true;
    }
}


void TerrainQuadtree::SubdivideNode(QuadTreeNode* node) 
{
    DirectX::XMFLOAT3 center = node->bounds.Center;
    DirectX::XMFLOAT3 extents = node->bounds.Extents;

    for (int i = 0; i < 4; ++i) 
    {
        node->children[i] = new QuadTreeNode();

        DirectX::XMFLOAT3 childCenter = center;
        DirectX::XMFLOAT3 childExtents = { extents.x * 0.5f, extents.y, extents.z * 0.5f };

        switch (i) 
        {
        case 0:
            childCenter.x -= extents.x * 0.5f;
            childCenter.z += extents.z * 0.5f;
            break;
        case 1:
            childCenter.x += extents.x * 0.5f;
            childCenter.z += extents.z * 0.5f;
            break;
        case 2:
            childCenter.x -= extents.x * 0.5f;
            childCenter.z -= extents.z * 0.5f;
            break;
        case 3:
            childCenter.x += extents.x * 0.5f;
            childCenter.z -= extents.z * 0.5f;
            break;
        }

        node->children[i]->bounds.Center = childCenter;
        node->children[i]->bounds.Extents = childExtents;
    }
}


void TerrainQuadtree::Update(const DirectX::XMMATRIX& viewProj, const DirectX::BoundingFrustum& cameraFrustum) 
{
    m_NodesToRender.clear();
    m_DebugNodesToRender.clear();
    UpdateNode(m_root, cameraFrustum);
}

void TerrainQuadtree::UpdateNode(QuadTreeNode* node, const DirectX::BoundingFrustum& cameraFrustum) 
{
    if (!node) return;

    DirectX::ContainmentType containment = cameraFrustum.Contains(node->bounds);
    if (containment == DirectX::DISJOINT) 
    {
        return;
    }

    // Calculate distance
    DirectX::XMVECTOR centerVec = DirectX::XMLoadFloat3(&node->bounds.Center);
    DirectX::XMVECTOR cameraPos = XMLoadFloat3(&cameraFrustum.Origin);
    DirectX::XMVECTOR distVec = DirectX::XMVector3Length(DirectX::XMVectorSubtract(centerVec, cameraPos));
    DirectX::XMFLOAT3 resultDist;
    DirectX::XMStoreFloat3(&resultDist, distVec);
    if (resultDist.x > distances[node->depth])
    {
        m_NodesToRender.push_back(node);
        m_DebugNodesToRender.push_back(node);
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            UpdateNode(node->children[i], cameraFrustum);
        }
    }
}
#include "SponzaLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


bool LoadModel(const std::string& path, Model& model, ID3D12Device* device,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    UINT mCbvSrvDescriptorSize, Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24])
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_ConvertToLeftHanded |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return false;
    }

    int meshIndex = 0;
    int materialIndex = 0;

    ProcessNode(scene->mRootNode, scene, model, device, path, meshIndex, geometries, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload);
    return true;
}

void ProcessNode(aiNode* node, const aiScene* scene, Model& model,
    ID3D12Device* device, const std::string& modelPath, int& index,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24])
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.push_back(ProcessMesh(mesh, scene, device, modelPath, model, index, geometries, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload));
        index++;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, model, device, modelPath, index, geometries, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload);
    }
}

Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene,
    ID3D12Device* device, const std::string& modelPath,
    Model& model, int& meshIndex,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24])
{
    Mesh result;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    DirectX::BoundingBox bounds;
    std::vector<DirectX::XMFLOAT3> points;

    // Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Pos = { mesh->mVertices[i].x / 70, mesh->mVertices[i].y / 70, mesh->mVertices[i].z / 70 };
        points.push_back(vertex.Pos);
        vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

        if (mesh->mTextureCoords[0])
        {
            vertex.TexC = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else
        {
            vertex.TexC = { 0.0f, 0.0f };
        }

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.TangentU = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
        }

        vertices.push_back(vertex);
    }
    const DirectX::XMFLOAT3* ptr = points.data();
    DirectX::BoundingBox::CreateFromPoints(bounds, points.size(), ptr, size_t(sizeof(DirectX::XMFLOAT3)));
    result.boundingBox = bounds;

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Buffer creation
    SubmeshGeometry meshSubmesh;
    meshSubmesh.IndexCount = indices.size();
    meshSubmesh.StartIndexLocation = 0;
    meshSubmesh.BaseVertexLocation = 0;
    meshSubmesh.VertexCount = vertices.size();

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "Sponza" + std::to_string(meshIndex);

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["mesh"] = meshSubmesh;

    result.vertexBufferView = geo->VertexBufferView();
    result.indexBufferView = geo->IndexBufferView();
    result.indexBuffer = geo->IndexBufferGPU.Get();
    result.vertexBuffer = geo->VertexBufferGPU.Get();

    result.vertices = vertices;
    result.indices = indices;

    geometries[geo->Name] = std::move(geo);

    int idDiffuse = -1;
    int idNormal = -1;

    // Materials and textures
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        idDiffuse = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse",
            result, scene, device, modelPath, model, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload);
        idNormal = LoadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal",
            result, scene, device, modelPath, model, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload);
    }

    result.materialIndex = idDiffuse;

    return result;
}

int LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
    const std::string& typeName, Mesh& mesh,
    const aiScene* scene, ID3D12Device* device,
    const std::string& modelPath,
    Model& model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24])
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        std::string texturePath = "Sponza/" + (std::string)str.C_Str();

        if (model.loadedTextures.find(texturePath) != model.loadedTextures.end())
        {
            if (type == aiTextureType_DIFFUSE)
            {
                mesh.diffuseTexture = model.loadedTextures[texturePath];
            }
            else if (type == aiTextureType_NORMALS)
            {
                mesh.normalTexture = model.loadedTextures[texturePath];
            }
            skip = true;
        }

        if (!skip)
        {
            LoadSponzaTexture(device, texturePath, commandList, sponzaSrvHeap, materialIndex, mCbvSrvDescriptorSize, sponzaTextures, sponzaTexturesUpload);
            model.loadedTextures[texturePath] = sponzaTextures[materialIndex - 1];
            model.loadedTexturesIDs[texturePath] = materialIndex - 1;

            if (type == aiTextureType_DIFFUSE)
            {
                mesh.diffuseTexture = sponzaTextures[materialIndex - 1];
            }
            else if (type == aiTextureType_NORMALS)
            {
                mesh.normalTexture = sponzaTextures[materialIndex - 1];
            }
        }

        return model.loadedTexturesIDs[texturePath];
    }
}

void LoadSponzaTexture(ID3D12Device* device, const std::string& path,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24])
{
    int width, height, channels;
    unsigned char* imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!imageData)
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    // Создание текстуры
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&sponzaTextures[materialIndex]));

    // tmp buffer
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(sponzaTextures[materialIndex].Get(), 0, 1);

    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&sponzaTexturesUpload[materialIndex]));

    // Copy data to texture
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = imageData;
    textureData.RowPitch = width * 4;
    textureData.SlicePitch = textureData.RowPitch * height;

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        sponzaTextures[materialIndex].Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    commandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(commandList.Get(),
        sponzaTextures[materialIndex].Get(),
        sponzaTexturesUpload[materialIndex].Get(),
        0, 0, 1, &textureData);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        sponzaTextures[materialIndex].Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier2);

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(sponzaSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(materialIndex, mCbvSrvDescriptorSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;
    device->CreateShaderResourceView(sponzaTextures[materialIndex].Get(), &srvDesc, hDescriptor);

    materialIndex++;

    stbi_image_free(imageData);
}

#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXMath.h>
#include <vector>
#include "FrameResource.h"

struct Mesh 
{
    int materialIndex;
    DirectX::BoundingBox boundingBox;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> diffuseTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> normalTexture;
};

struct Model 
{
    std::vector<Mesh> meshes;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>> loadedTextures;
    std::unordered_map<std::string, int> loadedTexturesIDs;
};

void ProcessNode(aiNode* node, const aiScene* scene, Model& model,
    ID3D12Device* device, const std::string& modelPath, int& index,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24]);

Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene,
    ID3D12Device* device, const std::string& modelPath,
    Model& model, int& meshIndex,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24]);

int LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
    const std::string& typeName, Mesh& mesh,
    const aiScene* scene, ID3D12Device* device,
    const std::string& modelPath,
    Model& model, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24]);

void LoadSponzaTexture(ID3D12Device* device, const std::string& path,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    int& materialIndex, UINT mCbvSrvDescriptorSize,
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24]);

bool LoadModel(const std::string& path, Model& model, ID3D12Device* device,
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sponzaSrvHeap,
    UINT mCbvSrvDescriptorSize, Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTextures[24],
    Microsoft::WRL::ComPtr<ID3D12Resource> sponzaTexturesUpload[24]);


#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include <vector>
#include <wrl.h>
#include "Noise3DGenerator.h"

using Microsoft::WRL::ComPtr;

struct MarchingCubeVertex 
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};

class MarchingCubes {
public:
    MarchingCubes() {}
    MarchingCubes(UINT width, UINT height, UINT depth, float cellSize);
    ~MarchingCubes();

    bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList,
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries);
    void Update(float time);
    void GenerateGeometry(float isoLevel = 0.0f);

    void Create3DTexture(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void Update3DTexture(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
    void CreateVertexBuffer(std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& geometries,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

    std::vector<MarchingCubeVertex> m_vertices;

private:
    void CreateBuffers();
    float SampleDensity(uint32_t x, uint32_t y, uint32_t z);
    void CalculateNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2,
        const DirectX::XMFLOAT3& v3, DirectX::XMFLOAT3& normal);
    DirectX::XMFLOAT3 InterpolateVertex(int x1, int y1, int z1, int x2, int y2, int z2,
        float val1, float val2, float isoLevel);

    static const int edgeTable[256];
    static const int triTable[256][16];

    UINT m_width, m_height, m_depth;
    float m_cellSize;

    ID3D12Device* m_device;

    ComPtr<ID3D12Resource> m_volumeTexture;
    ComPtr<ID3D12Resource> m_volumeUploadBuffer;

    std::vector<float> m_densityData;

    Noise3DGenerator m_noiseGenerator;

    float m_time;
};


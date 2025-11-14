#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "d3dUtil.h"
#include "D3D12Engine.h"
#include "FrameResource.h"
#include <vector>
#include <string>

class GeometryManager
{
public:
    static std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    static std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    static std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    static std::vector<std::unique_ptr<RenderItem>> mAllRitems;
};


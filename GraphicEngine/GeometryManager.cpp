#include "GeometryManager.h"
#include "SponzaLoader.h"

Model GeometryManager::Sponza;
TerrainQuadtree GeometryManager::m_terrainQuadTree;

Microsoft::WRL::ComPtr<ID3D12Resource> GeometryManager::instanceOcTreeBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> GeometryManager::instanceOcTreeUploadBuffer;

Microsoft::WRL::ComPtr<ID3D12Resource> GeometryManager::sponzaTextures[24];
Microsoft::WRL::ComPtr<ID3D12Resource> GeometryManager::sponzaTexturesUpload[24];

std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> GeometryManager::mGeometries = std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>();
std::unordered_map<std::string, std::unique_ptr<Material>> GeometryManager::mMaterials = std::unordered_map<std::string, std::unique_ptr<Material>>();
std::unordered_map<std::string, std::unique_ptr<Texture>> GeometryManager::mTextures = std::unordered_map<std::string, std::unique_ptr<Texture>>();
std::vector<std::unique_ptr<RenderItem>> GeometryManager::mAllRitems = std::vector<std::unique_ptr<RenderItem>>();

std::vector<RenderItem*> GeometryManager::mRitemLayer[(int)RenderLayer::Count];

void GeometryManager::BuildMaterials()
{
    auto metalAnimate = std::make_unique<Material>();
    metalAnimate->Name = "metalAnimate";
    metalAnimate->MatCBIndex = 0;
    metalAnimate->DiffuseSrvHeapIndex = 1;
    metalAnimate->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    metalAnimate->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
    metalAnimate->Roughness = 0.2f;

    GeometryManager::mMaterials["metalAnimate"] = std::move(metalAnimate);

    auto tileCrate = std::make_unique<Material>();
    tileCrate->Name = "tileCrate";
    tileCrate->MatCBIndex = 1;
    tileCrate->DiffuseSrvHeapIndex = 1;
    tileCrate->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    tileCrate->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
    tileCrate->Roughness = 0.2f;

    GeometryManager::mMaterials["tileCrate"] = std::move(tileCrate);

    auto stoneMat = std::make_unique<Material>();
    stoneMat->Name = "stoneMaterial";
    stoneMat->MatCBIndex = 2;
    stoneMat->DiffuseSrvHeapIndex = 6;
    stoneMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    stoneMat->FresnelR0 = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
    stoneMat->Roughness = 0.5f;

    GeometryManager::mMaterials["stoneMaterial"] = std::move(stoneMat);

    auto planetMat = std::make_unique<Material>();
    planetMat->Name = "planetMaterial";
    planetMat->MatCBIndex = 3;
    planetMat->DiffuseSrvHeapIndex = 17;
    planetMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    planetMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    planetMat->Roughness = 0.125f;

    GeometryManager::mMaterials["planetMaterial"] = std::move(planetMat);

    auto particleMat = std::make_unique<Material>();
    particleMat->Name = "particleMaterial";
    particleMat->MatCBIndex = 4;
    particleMat->DiffuseSrvHeapIndex = 21;
    particleMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particleMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particleMat->Roughness = 0.125f;

    GeometryManager::mMaterials["particleMaterial"] = std::move(particleMat);

    auto particle2Mat = std::make_unique<Material>();
    particle2Mat->Name = "particle2Material";
    particle2Mat->MatCBIndex = 4;
    particle2Mat->DiffuseSrvHeapIndex = 22;
    particle2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    particle2Mat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    particle2Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["particle2Material"] = std::move(particle2Mat);

    auto skyboxMat = std::make_unique<Material>();
    skyboxMat->Name = "skyboxMaterial";
    skyboxMat->MatCBIndex = 5;
    skyboxMat->DiffuseSrvHeapIndex = 18;
    skyboxMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    skyboxMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    skyboxMat->Roughness = 0.125f;

    GeometryManager::mMaterials["skyboxMaterial"] = std::move(skyboxMat);

    auto smokeMat = std::make_unique<Material>();
    smokeMat->Name = "smokeMaterial";
    smokeMat->MatCBIndex = 6;
    smokeMat->DiffuseSrvHeapIndex = 23;
    smokeMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    smokeMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    smokeMat->Roughness = 0.125f;

    GeometryManager::mMaterials["smokeMaterial"] = std::move(smokeMat);

    auto bonfireMat = std::make_unique<Material>();
    bonfireMat->Name = "bonfireMaterial";
    bonfireMat->MatCBIndex = 7;
    bonfireMat->DiffuseSrvHeapIndex = 24;
    bonfireMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    bonfireMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    bonfireMat->Roughness = 0.125f;

    GeometryManager::mMaterials["bonfireMaterial"] = std::move(bonfireMat);

    auto grassMat = std::make_unique<Material>();
    grassMat->Name = "grassMaterial";
    grassMat->MatCBIndex = 8;
    grassMat->DiffuseSrvHeapIndex = 25;
    grassMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    grassMat->FresnelR0 = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
    grassMat->Roughness = 0.125f;

    GeometryManager::mMaterials["grassMaterial"] = std::move(grassMat);

    auto PBR1Mat = std::make_unique<Material>();
    PBR1Mat->Name = "PBR1Material";
    PBR1Mat->MatCBIndex = 9;
    PBR1Mat->DiffuseSrvHeapIndex = 23;
    PBR1Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR1Mat->FresnelR0 = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    PBR1Mat->Roughness = 0.125f;
    PBR1Mat->IsMetallic = 1;

    GeometryManager::mMaterials["PBR1Material"] = std::move(PBR1Mat);

    auto PBR2Mat = std::make_unique<Material>();
    PBR2Mat->Name = "PBR2Material";
    PBR2Mat->MatCBIndex = 10;
    PBR2Mat->DiffuseSrvHeapIndex = 28;
    PBR2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR2Mat->FresnelR0 = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    PBR2Mat->Roughness = 0.125f;
    PBR2Mat->IsMetallic = 1;

    GeometryManager::mMaterials["PBR2Material"] = std::move(PBR2Mat);

    auto PBR3Mat = std::make_unique<Material>();
    PBR3Mat->Name = "PBR3Material";
    PBR3Mat->MatCBIndex = 11;
    PBR3Mat->DiffuseSrvHeapIndex = 33;
    PBR3Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR3Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR3Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR3Material"] = std::move(PBR3Mat);

    auto PBR4Mat = std::make_unique<Material>();
    PBR4Mat->Name = "PBR4Material";
    PBR4Mat->MatCBIndex = 12;
    PBR4Mat->DiffuseSrvHeapIndex = 38;
    PBR4Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR4Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR4Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR4Material"] = std::move(PBR4Mat);

    auto PBR5Mat = std::make_unique<Material>();
    PBR5Mat->Name = "PBR5Material";
    PBR5Mat->MatCBIndex = 13;
    PBR5Mat->DiffuseSrvHeapIndex = 43;
    PBR5Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    PBR5Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    PBR5Mat->Roughness = 0.125f;

    GeometryManager::mMaterials["PBR5Material"] = std::move(PBR5Mat);

    auto TerrainMat = std::make_unique<Material>();
    TerrainMat->Name = "TerrainMaterial";
    TerrainMat->MatCBIndex = 14;
    TerrainMat->DiffuseSrvHeapIndex = 54;
    TerrainMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    TerrainMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    TerrainMat->Roughness = 0.9f;

    GeometryManager::mMaterials["TerrainMaterial"] = std::move(TerrainMat);

    auto RainMat = std::make_unique<Material>();
    RainMat->Name = "RainMaterial";
    RainMat->MatCBIndex = 15;
    RainMat->DiffuseSrvHeapIndex = 27;
    RainMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    RainMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    RainMat->Roughness = 0.9f;

    GeometryManager::mMaterials["RainMaterial"] = std::move(RainMat);

    auto Terrain2Mat = std::make_unique<Material>();
    Terrain2Mat->Name = "TerrainMaterial2";
    Terrain2Mat->MatCBIndex = 16;
    Terrain2Mat->DiffuseSrvHeapIndex = 59;
    Terrain2Mat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    Terrain2Mat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    Terrain2Mat->Roughness = 0.9f;

    GeometryManager::mMaterials["TerrainMaterial2"] = std::move(Terrain2Mat);

    auto HamburgerMat = std::make_unique<Material>();
    HamburgerMat->Name = "HamburgerMaterial";
    HamburgerMat->MatCBIndex = 17;
    HamburgerMat->DiffuseSrvHeapIndex = 62;
    HamburgerMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    HamburgerMat->FresnelR0 = DirectX::XMFLOAT3(0.04f, 0.04f, 0.04f);
    HamburgerMat->Roughness = 0.9f;

    GeometryManager::mMaterials["HamburgerMaterial"] = std::move(HamburgerMat);
}

void GeometryManager::BuildRenderItems()
{
    auto boxRitem = std::make_unique<RenderItem>();
    boxRitem->ObjCBIndex = 0;
    boxRitem->World = MathHelper::Identity4x4();
    boxRitem->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    boxRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxRitem));

    auto boxTileRitem = std::make_unique<RenderItem>();
    boxTileRitem->ObjCBIndex = 1;
    XMStoreFloat4x4(&boxTileRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(2.0f, 2.0f, 0.0f));
    boxTileRitem->Mat = GeometryManager::mMaterials["tileCrate"].get();
    boxTileRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxTileRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxTileRitem->IndexCount = boxTileRitem->Geo->DrawArgs["box2"].IndexCount;
    boxTileRitem->StartIndexLocation = boxTileRitem->Geo->DrawArgs["box2"].StartIndexLocation;
    boxTileRitem->BaseVertexLocation = boxTileRitem->Geo->DrawArgs["box2"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxTileRitem));


    auto stoneTessellationRitem = std::make_unique<RenderItem>();
    stoneTessellationRitem->ObjCBIndex = 2;
    XMStoreFloat4x4(&stoneTessellationRitem->World, DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(-3.0f, 0.0f, 0.0f));
    stoneTessellationRitem->TexTransform = MathHelper::Identity4x4();
    stoneTessellationRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    stoneTessellationRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    stoneTessellationRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    stoneTessellationRitem->IndexCount = stoneTessellationRitem->Geo->DrawArgs["box3"].IndexCount;
    stoneTessellationRitem->StartIndexLocation = stoneTessellationRitem->Geo->DrawArgs["box3"].StartIndexLocation;
    stoneTessellationRitem->BaseVertexLocation = stoneTessellationRitem->Geo->DrawArgs["box3"].BaseVertexLocation;
    mRitemLayer[(int)RenderLayer::Scene11].push_back(stoneTessellationRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(stoneTessellationRitem));


    auto boxInstancingRitem = std::make_unique<RenderItem>();
    boxInstancingRitem->ObjCBIndex = 3;
    boxInstancingRitem->World = MathHelper::Identity4x4();
    boxInstancingRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    boxInstancingRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxInstancingRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxInstancingRitem->IndexCount = boxInstancingRitem->Geo->DrawArgs["box4"].IndexCount;
    boxInstancingRitem->StartIndexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].StartIndexLocation;
    boxInstancingRitem->BaseVertexLocation = boxInstancingRitem->Geo->DrawArgs["box4"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxInstancingRitem));

    auto boxFPSRitem = std::make_unique<RenderItem>();
    boxFPSRitem->ObjCBIndex = 4;
    boxFPSRitem->World = MathHelper::Identity4x4();
    boxFPSRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    boxFPSRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    boxFPSRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    boxFPSRitem->IndexCount = boxFPSRitem->Geo->DrawArgs["boxFPS"].IndexCount;
    boxFPSRitem->StartIndexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].StartIndexLocation;
    boxFPSRitem->BaseVertexLocation = boxFPSRitem->Geo->DrawArgs["boxFPS"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(boxFPSRitem));

    auto pointLight1Ritem = std::make_unique<RenderItem>();
    pointLight1Ritem->ObjCBIndex = 5;
    pointLight1Ritem->World = MathHelper::Identity4x4();
    pointLight1Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight1Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight1Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight1Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight1Ritem->IndexCount = pointLight1Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    pointLight1Ritem->StartIndexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    pointLight1Ritem->BaseVertexLocation = pointLight1Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight1Ritem));

    auto pointLight2Ritem = std::make_unique<RenderItem>();
    pointLight2Ritem->ObjCBIndex = 6;
    pointLight2Ritem->World = MathHelper::Identity4x4();
    pointLight2Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight2Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight2Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight2Ritem->IndexCount = pointLight2Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    pointLight2Ritem->StartIndexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    pointLight2Ritem->BaseVertexLocation = pointLight2Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight2Ritem));

    auto pointLight3Ritem = std::make_unique<RenderItem>();
    pointLight3Ritem->ObjCBIndex = 7;
    pointLight3Ritem->World = MathHelper::Identity4x4();
    pointLight3Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight3Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight3Ritem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    pointLight3Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    pointLight3Ritem->IndexCount = pointLight3Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    pointLight3Ritem->StartIndexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    pointLight3Ritem->BaseVertexLocation = pointLight3Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(pointLight3Ritem));

    auto gridRitem = std::make_unique<RenderItem>();
    gridRitem->ObjCBIndex = 8;
    gridRitem->World = MathHelper::Identity4x4();
    gridRitem->TexTransform = MathHelper::Identity4x4();
    gridRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    gridRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(gridRitem));

    auto coneRitem = std::make_unique<RenderItem>();
    coneRitem->ObjCBIndex = 9;
    coneRitem->World = MathHelper::Identity4x4();
    coneRitem->TexTransform = MathHelper::Identity4x4();
    coneRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    coneRitem->Geo = GeometryManager::mGeometries["boxGeo"].get();
    coneRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    coneRitem->IndexCount = coneRitem->Geo->DrawArgs["cone"].IndexCount;
    coneRitem->StartIndexLocation = coneRitem->Geo->DrawArgs["cone"].StartIndexLocation;
    coneRitem->BaseVertexLocation = coneRitem->Geo->DrawArgs["cone"].BaseVertexLocation;
    GeometryManager::mAllRitems.push_back(std::move(coneRitem));

    auto planetSpritesRitem = std::make_unique<RenderItem>();
    planetSpritesRitem->World = MathHelper::Identity4x4();
    planetSpritesRitem->ObjCBIndex = 10;
    planetSpritesRitem->Mat = GeometryManager::mMaterials["planetMaterial"].get();
    planetSpritesRitem->Geo = GeometryManager::mGeometries["planetSpritesGeo"].get();
    planetSpritesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    planetSpritesRitem->IndexCount = planetSpritesRitem->Geo->DrawArgs["points"].IndexCount;
    planetSpritesRitem->StartIndexLocation = planetSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
    planetSpritesRitem->BaseVertexLocation = planetSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::BillboardSprites].push_back(planetSpritesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(planetSpritesRitem));

    auto particleRitem = std::make_unique<RenderItem>();
    particleRitem->World = MathHelper::Identity4x4();
    particleRitem->ObjCBIndex = 11;
    particleRitem->Mat = GeometryManager::mMaterials["particleMaterial"].get();
    particleRitem->Geo = GeometryManager::mGeometries["particlesGeo1"].get();
    particleRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particleRitem->IndexCount = particleRitem->Geo->DrawArgs["points1"].IndexCount;
    particleRitem->StartIndexLocation = particleRitem->Geo->DrawArgs["points1"].StartIndexLocation;
    particleRitem->BaseVertexLocation = particleRitem->Geo->DrawArgs["points1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles1].push_back(particleRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(particleRitem));

    auto skyboxRitem = std::make_unique<RenderItem>();
    skyboxRitem->World = MathHelper::Identity4x4();
    skyboxRitem->ObjCBIndex = 12;
    skyboxRitem->Mat = GeometryManager::mMaterials["skyboxMaterial"].get();
    skyboxRitem->Geo = GeometryManager::mGeometries["skyboxGeo"].get();
    skyboxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    skyboxRitem->IndexCount = skyboxRitem->Geo->DrawArgs["skybox"].IndexCount;
    skyboxRitem->StartIndexLocation = skyboxRitem->Geo->DrawArgs["skybox"].StartIndexLocation;
    skyboxRitem->BaseVertexLocation = skyboxRitem->Geo->DrawArgs["skybox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Sky].push_back(skyboxRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(skyboxRitem));

    auto instancingRitemScene3 = std::make_unique<RenderItem>();
    instancingRitemScene3->World = MathHelper::Identity4x4();
    instancingRitemScene3->ObjCBIndex = 13;
    instancingRitemScene3->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    instancingRitemScene3->Geo = GeometryManager::mGeometries["scene3Geo"].get();
    instancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    instancingRitemScene3->IndexCount = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
    instancingRitemScene3->StartIndexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
    instancingRitemScene3->BaseVertexLocation = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;
    instancingRitemScene3->Bounds = instancingRitemScene3->Geo->DrawArgs["boxInstancing"].Bounds;

    mRitemLayer[(int)RenderLayer::Scene3].push_back(instancingRitemScene3.get());
    GeometryManager::mAllRitems.push_back(std::move(instancingRitemScene3));

    for (int i = 0; i < 729; ++i)
    {
        auto nonInstancingRitemScene3 = std::make_unique<RenderItem>();
        nonInstancingRitemScene3->World = MathHelper::Identity4x4();
        nonInstancingRitemScene3->ObjCBIndex = 14 + i;
        nonInstancingRitemScene3->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
        nonInstancingRitemScene3->Geo = GeometryManager::mGeometries["scene3Geo"].get();
        nonInstancingRitemScene3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        nonInstancingRitemScene3->IndexCount = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].IndexCount;
        nonInstancingRitemScene3->StartIndexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].StartIndexLocation;
        nonInstancingRitemScene3->BaseVertexLocation = nonInstancingRitemScene3->Geo->DrawArgs["boxInstancing"].BaseVertexLocation;

        mRitemLayer[(int)RenderLayer::Scene3].push_back(nonInstancingRitemScene3.get());
        GeometryManager::mAllRitems.push_back(std::move(nonInstancingRitemScene3));
    }

    auto animateRitemScene4 = std::make_unique<RenderItem>();
    animateRitemScene4->World = MathHelper::Identity4x4();
    animateRitemScene4->ObjCBIndex = 743;
    animateRitemScene4->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    animateRitemScene4->Geo = GeometryManager::mGeometries["scene4Geo"].get();
    animateRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    animateRitemScene4->IndexCount = animateRitemScene4->Geo->DrawArgs["animateBox"].IndexCount;
    animateRitemScene4->StartIndexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].StartIndexLocation;
    animateRitemScene4->BaseVertexLocation = animateRitemScene4->Geo->DrawArgs["animateBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(animateRitemScene4.get());
    GeometryManager::mAllRitems.push_back(std::move(animateRitemScene4));

    auto tilingRitemScene4 = std::make_unique<RenderItem>();
    tilingRitemScene4->World = MathHelper::Identity4x4();
    tilingRitemScene4->ObjCBIndex = 744;
    tilingRitemScene4->Mat = GeometryManager::mMaterials["tileCrate"].get();
    tilingRitemScene4->Geo = GeometryManager::mGeometries["scene4Geo"].get();
    tilingRitemScene4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tilingRitemScene4->IndexCount = tilingRitemScene4->Geo->DrawArgs["tilingBox"].IndexCount;
    tilingRitemScene4->StartIndexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].StartIndexLocation;
    tilingRitemScene4->BaseVertexLocation = tilingRitemScene4->Geo->DrawArgs["tilingBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene4].push_back(tilingRitemScene4.get());
    GeometryManager::mAllRitems.push_back(std::move(tilingRitemScene4));

    auto tessellationRitemScene5 = std::make_unique<RenderItem>();
    tessellationRitemScene5->World = MathHelper::Identity4x4();
    tessellationRitemScene5->ObjCBIndex = 745;
    tessellationRitemScene5->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    tessellationRitemScene5->Geo = GeometryManager::mGeometries["scene5Geo"].get();
    tessellationRitemScene5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    tessellationRitemScene5->IndexCount = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].IndexCount;
    tessellationRitemScene5->StartIndexLocation = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].StartIndexLocation;
    tessellationRitemScene5->BaseVertexLocation = tessellationRitemScene5->Geo->DrawArgs["tessellationBox"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene5].push_back(tessellationRitemScene5.get());
    GeometryManager::mAllRitems.push_back(std::move(tessellationRitemScene5));

    auto particle2Ritem = std::make_unique<RenderItem>();
    particle2Ritem->World = MathHelper::Identity4x4();
    particle2Ritem->ObjCBIndex = 746;
    particle2Ritem->Mat = GeometryManager::mMaterials["particle2Material"].get();
    particle2Ritem->Geo = GeometryManager::mGeometries["particlesGeo2"].get();
    particle2Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particle2Ritem->IndexCount = particle2Ritem->Geo->DrawArgs["points2"].IndexCount;
    particle2Ritem->StartIndexLocation = particle2Ritem->Geo->DrawArgs["points2"].StartIndexLocation;
    particle2Ritem->BaseVertexLocation = particle2Ritem->Geo->DrawArgs["points2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles2].push_back(particle2Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(particle2Ritem));

    auto sponzaRitem = std::make_unique<RenderItem>();
    sponzaRitem->World = MathHelper::Identity4x4();
    sponzaRitem->ObjCBIndex = 747;
    sponzaRitem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    sponzaRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    GeometryManager::mAllRitems.push_back(std::move(sponzaRitem));

    auto bonfireRitem = std::make_unique<RenderItem>();
    bonfireRitem->World = MathHelper::Identity4x4();
    bonfireRitem->ObjCBIndex = 748;
    bonfireRitem->Mat = GeometryManager::mMaterials["bonfireMaterial"].get();
    bonfireRitem->Geo = GeometryManager::mGeometries["scene6Bonfire"].get();
    bonfireRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    bonfireRitem->IndexCount = bonfireRitem->Geo->DrawArgs["Bonfire"].IndexCount;
    bonfireRitem->StartIndexLocation = bonfireRitem->Geo->DrawArgs["Bonfire"].StartIndexLocation;
    bonfireRitem->BaseVertexLocation = bonfireRitem->Geo->DrawArgs["Bonfire"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene6_3].push_back(bonfireRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(bonfireRitem));

    auto grassPlatformRitem = std::make_unique<RenderItem>();
    grassPlatformRitem->World = MathHelper::Identity4x4();
    grassPlatformRitem->ObjCBIndex = 749;
    grassPlatformRitem->Mat = GeometryManager::mMaterials["grassMaterial"].get();
    grassPlatformRitem->Geo = GeometryManager::mGeometries["scene6GrassPlatform"].get();
    grassPlatformRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    grassPlatformRitem->IndexCount = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].IndexCount;
    grassPlatformRitem->StartIndexLocation = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].StartIndexLocation;
    grassPlatformRitem->BaseVertexLocation = grassPlatformRitem->Geo->DrawArgs["GrassPlatform"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene6_3].push_back(grassPlatformRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(grassPlatformRitem));

    auto particleSmokeRitem = std::make_unique<RenderItem>();
    particleSmokeRitem->World = MathHelper::Identity4x4();
    particleSmokeRitem->ObjCBIndex = 750;
    particleSmokeRitem->Mat = GeometryManager::mMaterials["smokeMaterial"].get();
    particleSmokeRitem->Geo = GeometryManager::mGeometries["particlesGeo3"].get();
    particleSmokeRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    particleSmokeRitem->IndexCount = particleSmokeRitem->Geo->DrawArgs["points3"].IndexCount;
    particleSmokeRitem->StartIndexLocation = particleSmokeRitem->Geo->DrawArgs["points3"].StartIndexLocation;
    particleSmokeRitem->BaseVertexLocation = particleSmokeRitem->Geo->DrawArgs["points3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Particles3].push_back(particleSmokeRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(particleSmokeRitem));

    auto pointLight1Scene10Ritem = std::make_unique<RenderItem>();
    pointLight1Scene10Ritem->ObjCBIndex = 751;
    pointLight1Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight1Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight1Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight1Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight1Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight1Scene10Ritem->IndexCount = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].IndexCount;
    pointLight1Scene10Ritem->StartIndexLocation = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].StartIndexLocation;
    pointLight1Scene10Ritem->BaseVertexLocation = pointLight1Scene10Ritem->Geo->DrawArgs["pointLight1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight1Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight1Scene10Ritem));

    auto pointLight2Scene10Ritem = std::make_unique<RenderItem>();
    pointLight2Scene10Ritem->ObjCBIndex = 752;
    pointLight2Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight2Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight2Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight2Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight2Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight2Scene10Ritem->IndexCount = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].IndexCount;
    pointLight2Scene10Ritem->StartIndexLocation = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].StartIndexLocation;
    pointLight2Scene10Ritem->BaseVertexLocation = pointLight2Scene10Ritem->Geo->DrawArgs["pointLight2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight2Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight2Scene10Ritem));

    auto pointLight3Scene10Ritem = std::make_unique<RenderItem>();
    pointLight3Scene10Ritem->ObjCBIndex = 753;
    pointLight3Scene10Ritem->World = MathHelper::Identity4x4();
    pointLight3Scene10Ritem->TexTransform = MathHelper::Identity4x4();
    pointLight3Scene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    pointLight3Scene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    pointLight3Scene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    pointLight3Scene10Ritem->IndexCount = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].IndexCount;
    pointLight3Scene10Ritem->StartIndexLocation = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].StartIndexLocation;
    pointLight3Scene10Ritem->BaseVertexLocation = pointLight3Scene10Ritem->Geo->DrawArgs["pointLight3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(pointLight3Scene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(pointLight3Scene10Ritem));

    auto coneLightScene10Ritem = std::make_unique<RenderItem>();
    coneLightScene10Ritem->ObjCBIndex = 754;
    coneLightScene10Ritem->World = MathHelper::Identity4x4();
    coneLightScene10Ritem->TexTransform = MathHelper::Identity4x4();
    coneLightScene10Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    coneLightScene10Ritem->Geo = GeometryManager::mGeometries["debugGeo"].get();
    coneLightScene10Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    coneLightScene10Ritem->IndexCount = coneLightScene10Ritem->Geo->DrawArgs["cone"].IndexCount;
    coneLightScene10Ritem->StartIndexLocation = coneLightScene10Ritem->Geo->DrawArgs["cone"].StartIndexLocation;
    coneLightScene10Ritem->BaseVertexLocation = coneLightScene10Ritem->Geo->DrawArgs["cone"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10DebugGeometry].push_back(coneLightScene10Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(coneLightScene10Ritem));

    auto box1Scene7Ritem = std::make_unique<RenderItem>();
    box1Scene7Ritem->ObjCBIndex = 755;
    box1Scene7Ritem->World = MathHelper::Identity4x4();
    box1Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box1Scene7Ritem->Mat = GeometryManager::mMaterials["metalAnimate"].get();
    box1Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box1Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box1Scene7Ritem->IndexCount = box1Scene7Ritem->Geo->DrawArgs["box1"].IndexCount;
    box1Scene7Ritem->StartIndexLocation = box1Scene7Ritem->Geo->DrawArgs["box1"].StartIndexLocation;
    box1Scene7Ritem->BaseVertexLocation = box1Scene7Ritem->Geo->DrawArgs["box1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box1Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box1Scene7Ritem));

    auto box2Scene7Ritem = std::make_unique<RenderItem>();
    box2Scene7Ritem->ObjCBIndex = 756;
    box2Scene7Ritem->World = MathHelper::Identity4x4();
    box2Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box2Scene7Ritem->Mat = GeometryManager::mMaterials["stoneMaterial"].get();
    box2Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box2Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box2Scene7Ritem->IndexCount = box2Scene7Ritem->Geo->DrawArgs["box2"].IndexCount;
    box2Scene7Ritem->StartIndexLocation = box2Scene7Ritem->Geo->DrawArgs["box2"].StartIndexLocation;
    box2Scene7Ritem->BaseVertexLocation = box2Scene7Ritem->Geo->DrawArgs["box2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box2Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box2Scene7Ritem));

    auto box3Scene7Ritem = std::make_unique<RenderItem>();
    box3Scene7Ritem->ObjCBIndex = 757;
    box3Scene7Ritem->World = MathHelper::Identity4x4();
    box3Scene7Ritem->TexTransform = MathHelper::Identity4x4();
    box3Scene7Ritem->Mat = GeometryManager::mMaterials["planetMaterial"].get();
    box3Scene7Ritem->Geo = GeometryManager::mGeometries["scene7Geo"].get();
    box3Scene7Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    box3Scene7Ritem->IndexCount = box3Scene7Ritem->Geo->DrawArgs["box3"].IndexCount;
    box3Scene7Ritem->StartIndexLocation = box3Scene7Ritem->Geo->DrawArgs["box3"].StartIndexLocation;
    box3Scene7Ritem->BaseVertexLocation = box3Scene7Ritem->Geo->DrawArgs["box3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene7].push_back(box3Scene7Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(box3Scene7Ritem));

    auto PBR1Scene9Ritem = std::make_unique<RenderItem>();
    PBR1Scene9Ritem->ObjCBIndex = 758;
    PBR1Scene9Ritem->World = MathHelper::Identity4x4();
    PBR1Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR1Scene9Ritem->Mat = GeometryManager::mMaterials["PBR1Material"].get();
    PBR1Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR1Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR1Scene9Ritem->IndexCount = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].IndexCount;
    PBR1Scene9Ritem->StartIndexLocation = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].StartIndexLocation;
    PBR1Scene9Ritem->BaseVertexLocation = PBR1Scene9Ritem->Geo->DrawArgs["sphere1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR1Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR1Scene9Ritem));

    auto PBR2Scene9Ritem = std::make_unique<RenderItem>();
    PBR2Scene9Ritem->ObjCBIndex = 759;
    PBR2Scene9Ritem->World = MathHelper::Identity4x4();
    PBR2Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR2Scene9Ritem->Mat = GeometryManager::mMaterials["PBR2Material"].get();
    PBR2Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR2Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR2Scene9Ritem->IndexCount = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].IndexCount;
    PBR2Scene9Ritem->StartIndexLocation = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].StartIndexLocation;
    PBR2Scene9Ritem->BaseVertexLocation = PBR2Scene9Ritem->Geo->DrawArgs["sphere2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR2Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR2Scene9Ritem));

    auto PBR3Scene9Ritem = std::make_unique<RenderItem>();
    PBR3Scene9Ritem->ObjCBIndex = 760;
    PBR3Scene9Ritem->World = MathHelper::Identity4x4();
    PBR3Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR3Scene9Ritem->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    PBR3Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR3Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR3Scene9Ritem->IndexCount = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].IndexCount;
    PBR3Scene9Ritem->StartIndexLocation = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].StartIndexLocation;
    PBR3Scene9Ritem->BaseVertexLocation = PBR3Scene9Ritem->Geo->DrawArgs["sphere3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR3Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR3Scene9Ritem));

    auto PBR4Scene9Ritem = std::make_unique<RenderItem>();
    PBR4Scene9Ritem->ObjCBIndex = 761;
    PBR4Scene9Ritem->World = MathHelper::Identity4x4();
    PBR4Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR4Scene9Ritem->Mat = GeometryManager::mMaterials["PBR4Material"].get();
    PBR4Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR4Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR4Scene9Ritem->IndexCount = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].IndexCount;
    PBR4Scene9Ritem->StartIndexLocation = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].StartIndexLocation;
    PBR4Scene9Ritem->BaseVertexLocation = PBR4Scene9Ritem->Geo->DrawArgs["sphere4"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR4Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR4Scene9Ritem));

    auto PBR5Scene9Ritem = std::make_unique<RenderItem>();
    PBR5Scene9Ritem->ObjCBIndex = 762;
    PBR5Scene9Ritem->World = MathHelper::Identity4x4();
    PBR5Scene9Ritem->TexTransform = MathHelper::Identity4x4();
    PBR5Scene9Ritem->Mat = GeometryManager::mMaterials["PBR5Material"].get();
    PBR5Scene9Ritem->Geo = GeometryManager::mGeometries["scene9Geo"].get();
    PBR5Scene9Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PBR5Scene9Ritem->IndexCount = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].IndexCount;
    PBR5Scene9Ritem->StartIndexLocation = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].StartIndexLocation;
    PBR5Scene9Ritem->BaseVertexLocation = PBR5Scene9Ritem->Geo->DrawArgs["sphere5"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9].push_back(PBR5Scene9Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(PBR5Scene9Ritem));

    auto sphere0LOD = std::make_unique<RenderItem>();
    sphere0LOD->ObjCBIndex = 763;
    sphere0LOD->World = MathHelper::Identity4x4();
    sphere0LOD->TexTransform = MathHelper::Identity4x4();
    sphere0LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere0LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere0LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere0LOD->IndexCount = sphere0LOD->Geo->DrawArgs["sphereLOD0"].IndexCount;
    sphere0LOD->StartIndexLocation = sphere0LOD->Geo->DrawArgs["sphereLOD0"].StartIndexLocation;
    sphere0LOD->BaseVertexLocation = sphere0LOD->Geo->DrawArgs["sphereLOD0"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere0LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere0LOD));

    auto sphere1LOD = std::make_unique<RenderItem>();
    sphere1LOD->ObjCBIndex = 764;
    sphere1LOD->World = MathHelper::Identity4x4();
    sphere1LOD->TexTransform = MathHelper::Identity4x4();
    sphere1LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere1LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere1LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere1LOD->IndexCount = sphere1LOD->Geo->DrawArgs["sphereLOD1"].IndexCount;
    sphere1LOD->StartIndexLocation = sphere1LOD->Geo->DrawArgs["sphereLOD1"].StartIndexLocation;
    sphere1LOD->BaseVertexLocation = sphere1LOD->Geo->DrawArgs["sphereLOD1"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere1LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere1LOD));

    auto sphere2LOD = std::make_unique<RenderItem>();
    sphere2LOD->ObjCBIndex = 765;
    sphere2LOD->World = MathHelper::Identity4x4();
    sphere2LOD->TexTransform = MathHelper::Identity4x4();
    sphere2LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere2LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere2LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere2LOD->IndexCount = sphere2LOD->Geo->DrawArgs["sphereLOD2"].IndexCount;
    sphere2LOD->StartIndexLocation = sphere2LOD->Geo->DrawArgs["sphereLOD2"].StartIndexLocation;
    sphere2LOD->BaseVertexLocation = sphere2LOD->Geo->DrawArgs["sphereLOD2"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere2LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere2LOD));

    auto sphere3LOD = std::make_unique<RenderItem>();
    sphere3LOD->ObjCBIndex = 766;
    sphere3LOD->World = MathHelper::Identity4x4();
    sphere3LOD->TexTransform = MathHelper::Identity4x4();
    sphere3LOD->Mat = GeometryManager::mMaterials["PBR3Material"].get();
    sphere3LOD->Geo = GeometryManager::mGeometries["scene3LODGeo"].get();
    sphere3LOD->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphere3LOD->IndexCount = sphere3LOD->Geo->DrawArgs["sphereLOD3"].IndexCount;
    sphere3LOD->StartIndexLocation = sphere3LOD->Geo->DrawArgs["sphereLOD3"].StartIndexLocation;
    sphere3LOD->BaseVertexLocation = sphere3LOD->Geo->DrawArgs["sphereLOD3"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene3LOD].push_back(sphere3LOD.get());
    GeometryManager::mAllRitems.push_back(std::move(sphere3LOD));

    auto sphereRMDemo = std::make_unique<RenderItem>();
    sphereRMDemo->ObjCBIndex = 767;
    sphereRMDemo->World = MathHelper::Identity4x4();
    sphereRMDemo->TexTransform = MathHelper::Identity4x4();
    sphereRMDemo->Mat = GeometryManager::mMaterials["PBR1Material"].get();
    sphereRMDemo->Geo = GeometryManager::mGeometries["scene9RMDemoGeo"].get();
    sphereRMDemo->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphereRMDemo->IndexCount = sphereRMDemo->Geo->DrawArgs["sphere"].IndexCount;
    sphereRMDemo->StartIndexLocation = sphereRMDemo->Geo->DrawArgs["sphere"].StartIndexLocation;
    sphereRMDemo->BaseVertexLocation = sphereRMDemo->Geo->DrawArgs["sphere"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene9RMDemo].push_back(sphereRMDemo.get());
    GeometryManager::mAllRitems.push_back(std::move(sphereRMDemo));

    auto terrainRitem = std::make_unique<RenderItem>();
    terrainRitem->ObjCBIndex = 768;
    terrainRitem->World = MathHelper::Identity4x4();
    terrainRitem->TexTransform = MathHelper::Identity4x4();
    terrainRitem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    terrainRitem->Geo = GeometryManager::mGeometries["scene12Geo"].get();
    terrainRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
    terrainRitem->IndexCount = terrainRitem->Geo->DrawArgs["plane"].IndexCount;
    terrainRitem->StartIndexLocation = terrainRitem->Geo->DrawArgs["plane"].StartIndexLocation;
    terrainRitem->BaseVertexLocation = terrainRitem->Geo->DrawArgs["plane"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene12].push_back(terrainRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(terrainRitem));

    auto moreLightRitem = std::make_unique<RenderItem>();
    moreLightRitem->ObjCBIndex = 769;
    moreLightRitem->World = MathHelper::Identity4x4();
    moreLightRitem->TexTransform = MathHelper::Identity4x4();
    moreLightRitem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    moreLightRitem->Geo = GeometryManager::mGeometries["moreLightGeo"].get();
    moreLightRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    moreLightRitem->IndexCount = moreLightRitem->Geo->DrawArgs["pointLight"].IndexCount;
    moreLightRitem->StartIndexLocation = moreLightRitem->Geo->DrawArgs["pointLight"].StartIndexLocation;
    moreLightRitem->BaseVertexLocation = moreLightRitem->Geo->DrawArgs["pointLight"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene10MoreLight].push_back(moreLightRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(moreLightRitem));

    auto sphereScene13Ritem = std::make_unique<RenderItem>();
    sphereScene13Ritem->ObjCBIndex = 770;
    sphereScene13Ritem->World = MathHelper::Identity4x4();
    sphereScene13Ritem->TexTransform = MathHelper::Identity4x4();
    sphereScene13Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    sphereScene13Ritem->Geo = GeometryManager::mGeometries["scene13Geo"].get();
    sphereScene13Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    sphereScene13Ritem->IndexCount = sphereScene13Ritem->Geo->DrawArgs["sphere"].IndexCount;
    sphereScene13Ritem->StartIndexLocation = sphereScene13Ritem->Geo->DrawArgs["sphere"].StartIndexLocation;
    sphereScene13Ritem->BaseVertexLocation = sphereScene13Ritem->Geo->DrawArgs["sphere"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene13].push_back(sphereScene13Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(sphereScene13Ritem));

    auto octreeScene13Ritem = std::make_unique<RenderItem>();
    octreeScene13Ritem->ObjCBIndex = 771;
    octreeScene13Ritem->World = MathHelper::Identity4x4();
    octreeScene13Ritem->TexTransform = MathHelper::Identity4x4();
    octreeScene13Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial"].get();
    octreeScene13Ritem->Geo = GeometryManager::mGeometries["scene13OctreeGeo"].get();
    octreeScene13Ritem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    octreeScene13Ritem->IndexCount = octreeScene13Ritem->Geo->DrawArgs["octree"].IndexCount;
    octreeScene13Ritem->StartIndexLocation = octreeScene13Ritem->Geo->DrawArgs["octree"].StartIndexLocation;
    octreeScene13Ritem->BaseVertexLocation = octreeScene13Ritem->Geo->DrawArgs["octree"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene13Octree].push_back(octreeScene13Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(octreeScene13Ritem));

    auto rainParticlesRitem = std::make_unique<RenderItem>();
    rainParticlesRitem->ObjCBIndex = 772;
    rainParticlesRitem->World = MathHelper::Identity4x4();
    rainParticlesRitem->TexTransform = MathHelper::Identity4x4();
    rainParticlesRitem->Mat = GeometryManager::mMaterials["RainMaterial"].get();
    rainParticlesRitem->Geo = GeometryManager::mGeometries["particlesGeo4"].get();
    rainParticlesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    rainParticlesRitem->IndexCount = rainParticlesRitem->Geo->DrawArgs["points4"].IndexCount;
    rainParticlesRitem->StartIndexLocation = rainParticlesRitem->Geo->DrawArgs["points4"].StartIndexLocation;
    rainParticlesRitem->BaseVertexLocation = rainParticlesRitem->Geo->DrawArgs["points4"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::RainParticles].push_back(rainParticlesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(rainParticlesRitem));

    auto terrain2Ritem = std::make_unique<RenderItem>();
    terrain2Ritem->ObjCBIndex = 773;
    terrain2Ritem->World = MathHelper::Identity4x4();
    terrain2Ritem->TexTransform = MathHelper::Identity4x4();
    terrain2Ritem->Mat = GeometryManager::mMaterials["TerrainMaterial2"].get();
    terrain2Ritem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

    mRitemLayer[(int)RenderLayer::Scene14].push_back(terrain2Ritem.get());
    GeometryManager::mAllRitems.push_back(std::move(terrain2Ritem));

    auto hamburgerRitem = std::make_unique<RenderItem>();
    hamburgerRitem->ObjCBIndex = 774;
    hamburgerRitem->World = MathHelper::Identity4x4();
    hamburgerRitem->TexTransform = MathHelper::Identity4x4();
    hamburgerRitem->Mat = GeometryManager::mMaterials["HamburgerMaterial"].get();
    hamburgerRitem->Geo = GeometryManager::mGeometries["scene15Geo"].get();
    hamburgerRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    hamburgerRitem->IndexCount = hamburgerRitem->Geo->DrawArgs["Hamburger"].IndexCount;
    hamburgerRitem->StartIndexLocation = hamburgerRitem->Geo->DrawArgs["Hamburger"].StartIndexLocation;
    hamburgerRitem->BaseVertexLocation = hamburgerRitem->Geo->DrawArgs["Hamburger"].BaseVertexLocation;

    mRitemLayer[(int)RenderLayer::Scene15].push_back(hamburgerRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(hamburgerRitem));

    auto marchingCubesRitem = std::make_unique<RenderItem>();
    marchingCubesRitem->ObjCBIndex = 775;
    marchingCubesRitem->World = MathHelper::Identity4x4();
    marchingCubesRitem->TexTransform = MathHelper::Identity4x4();
    marchingCubesRitem->Mat = GeometryManager::mMaterials["TerrainMaterial2"].get();
    marchingCubesRitem->Geo = GeometryManager::mGeometries["MarchingCubes"].get();
    marchingCubesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    mRitemLayer[(int)RenderLayer::Scene17].push_back(marchingCubesRitem.get());
    GeometryManager::mAllRitems.push_back(std::move(marchingCubesRitem));


    for (int i = 0; i < GeometryManager::mAllRitems.size(); ++i)
    {
        if (i < 5)
            mRitemLayer[(int)RenderLayer::Opaque].push_back(GeometryManager::mAllRitems[i].get());
        else if (i < 10)
            mRitemLayer[(int)RenderLayer::OpaqueWireframe].push_back(GeometryManager::mAllRitems[i].get());
    }
}

void GeometryManager::LoadTextures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList,
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice)
{
    auto metalAnimateTex = std::make_unique<Texture>();
    metalAnimateTex->Name = "MetalTex";
    metalAnimateTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalTex.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateTex->Filename.c_str(),
        metalAnimateTex->Resource, metalAnimateTex->UploadHeap);

    GeometryManager::mTextures[metalAnimateTex->Name] = std::move(metalAnimateTex);

    auto metalAnimateNorm = std::make_unique<Texture>();
    metalAnimateNorm->Name = "MetalNorm";
    metalAnimateNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalNorm.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateNorm->Filename.c_str(),
        metalAnimateNorm->Resource, metalAnimateNorm->UploadHeap);

    GeometryManager::mTextures[metalAnimateNorm->Name] = std::move(metalAnimateNorm);

    auto metalAnimateDisplacement = std::make_unique<Texture>();
    metalAnimateDisplacement->Name = "MetalDisplacement";
    metalAnimateDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalDisplacement.dds";
    DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAnimateDisplacement->Filename.c_str(),
        metalAnimateDisplacement->Resource, metalAnimateDisplacement->UploadHeap);

    GeometryManager::mTextures[metalAnimateDisplacement->Name] = std::move(metalAnimateDisplacement);

    auto metalRoughness = std::make_unique<Texture>();
    metalRoughness->Name = "MetalRoughness";
    metalRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalRoughness->Filename.c_str(),
        metalRoughness->Resource, metalRoughness->UploadHeap), true);

    GeometryManager::mTextures[metalRoughness->Name] = std::move(metalRoughness);

    auto metalAO = std::make_unique<Texture>();
    metalAO->Name = "MetalAO";
    metalAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), metalAO->Filename.c_str(),
        metalAO->Resource, metalAO->UploadHeap), true);

    GeometryManager::mTextures[metalAO->Name] = std::move(metalAO);

    auto stoneTex = std::make_unique<Texture>();
    stoneTex->Name = "StoneTex";
    stoneTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneTex->Filename.c_str(),
        stoneTex->Resource, stoneTex->UploadHeap), false);
    GeometryManager::mTextures[stoneTex->Name] = std::move(stoneTex);

    auto stoneNorm = std::make_unique<Texture>();
    stoneNorm->Name = "StoneNorm";
    stoneNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneNorm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneNorm->Filename.c_str(),
        stoneNorm->Resource, stoneNorm->UploadHeap), false);
    GeometryManager::mTextures[stoneNorm->Name] = std::move(stoneNorm);

    auto stoneDisplacement = std::make_unique<Texture>();
    stoneDisplacement->Name = "StoneDisplacement";
    stoneDisplacement->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneDisplacement.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneDisplacement->Filename.c_str(),
        stoneDisplacement->Resource, stoneDisplacement->UploadHeap), true);
    GeometryManager::mTextures[stoneDisplacement->Name] = std::move(stoneDisplacement);

    auto stoneRoughness = std::make_unique<Texture>();
    stoneRoughness->Name = "StoneRoughness";
    stoneRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneRoughness->Filename.c_str(),
        stoneRoughness->Resource, stoneRoughness->UploadHeap), true);
    GeometryManager::mTextures[stoneRoughness->Name] = std::move(stoneRoughness);

    auto stoneAO = std::make_unique<Texture>();
    stoneAO->Name = "StoneAO";
    stoneAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), stoneAO->Filename.c_str(),
        stoneAO->Resource, stoneAO->UploadHeap), true);
    GeometryManager::mTextures[stoneAO->Name] = std::move(stoneAO);

    auto planetTex = std::make_unique<Texture>();
    planetTex->Name = "PlanetTex";
    planetTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PlanetTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), planetTex->Filename.c_str(),
        planetTex->Resource, planetTex->UploadHeap), true);
    GeometryManager::mTextures[planetTex->Name] = std::move(planetTex);

    auto particleTex = std::make_unique<Texture>();
    particleTex->Name = "ParticleTex";
    particleTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\ParticleTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particleTex->Filename.c_str(),
        particleTex->Resource, particleTex->UploadHeap), true);
    GeometryManager::mTextures[particleTex->Name] = std::move(particleTex);

    auto particle2Tex = std::make_unique<Texture>();
    particle2Tex->Name = "Particle2Tex";
    particle2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Particle2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particle2Tex->Filename.c_str(),
        particle2Tex->Resource, particle2Tex->UploadHeap), true);
    GeometryManager::mTextures[particle2Tex->Name] = std::move(particle2Tex);

    auto shadow1Tex = std::make_unique<Texture>();
    shadow1Tex->Name = "Shadow1Tex";
    shadow1Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Shadow1Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow1Tex->Filename.c_str(),
        shadow1Tex->Resource, shadow1Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow1Tex->Name] = std::move(shadow1Tex);

    auto shadow2Tex = std::make_unique<Texture>();
    shadow2Tex->Name = "Shadow2Tex";
    shadow2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\StoneTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow2Tex->Filename.c_str(),
        shadow2Tex->Resource, shadow2Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow2Tex->Name] = std::move(shadow2Tex);

    auto shadow3Tex = std::make_unique<Texture>();
    shadow3Tex->Name = "Shadow3Tex";
    shadow3Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\MetalTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), shadow3Tex->Filename.c_str(),
        shadow3Tex->Resource, shadow3Tex->UploadHeap), true);
    GeometryManager::mTextures[shadow3Tex->Name] = std::move(shadow3Tex);

    auto bonfireTex = std::make_unique<Texture>();
    bonfireTex->Name = "BonfireTex";
    bonfireTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\BonfireTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), bonfireTex->Filename.c_str(),
        bonfireTex->Resource, bonfireTex->UploadHeap), true);
    GeometryManager::mTextures[bonfireTex->Name] = std::move(bonfireTex);

    auto particlesSmokeTex = std::make_unique<Texture>();
    particlesSmokeTex->Name = "ParticleSmokeTex";
    particlesSmokeTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\ParticleSmokeTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), particlesSmokeTex->Filename.c_str(),
        particlesSmokeTex->Resource, particlesSmokeTex->UploadHeap), true);
    GeometryManager::mTextures[particlesSmokeTex->Name] = std::move(particlesSmokeTex);

    auto grassTex = std::make_unique<Texture>();
    grassTex->Name = "GrassTex";
    grassTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\GrassTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), grassTex->Filename.c_str(),
        grassTex->Resource, grassTex->UploadHeap), true);
    GeometryManager::mTextures[grassTex->Name] = std::move(grassTex);

    auto skyboxTex = std::make_unique<Texture>();
    skyboxTex->Name = "SkyboxTex";
    skyboxTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\SkyboxTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), skyboxTex->Filename.c_str(),
        skyboxTex->Resource, skyboxTex->UploadHeap), true);
    GeometryManager::mTextures[skyboxTex->Name] = std::move(skyboxTex);

    auto PBR1Tex = std::make_unique<Texture>();
    PBR1Tex->Name = "PBR1Tex";
    PBR1Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Tex->Filename.c_str(),
        PBR1Tex->Resource, PBR1Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR1Tex->Name] = std::move(PBR1Tex);

    auto PBR1Norm = std::make_unique<Texture>();
    PBR1Norm->Name = "PBR1Norm";
    PBR1Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Norm->Filename.c_str(),
        PBR1Norm->Resource, PBR1Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR1Norm->Name] = std::move(PBR1Norm);

    auto PBR1AO = std::make_unique<Texture>();
    PBR1AO->Name = "PBR1AO";
    PBR1AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1AO->Filename.c_str(),
        PBR1AO->Resource, PBR1AO->UploadHeap), true);
    GeometryManager::mTextures[PBR1AO->Name] = std::move(PBR1AO);

    auto PBR1Metallic = std::make_unique<Texture>();
    PBR1Metallic->Name = "PBR1Metallic";
    PBR1Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Metallic->Filename.c_str(),
        PBR1Metallic->Resource, PBR1Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR1Metallic->Name] = std::move(PBR1Metallic);

    auto PBR1Roughness = std::make_unique<Texture>();
    PBR1Roughness->Name = "PBR1Roughness";
    PBR1Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR1Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR1Roughness->Filename.c_str(),
        PBR1Roughness->Resource, PBR1Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR1Roughness->Name] = std::move(PBR1Roughness);

    auto PBR2Tex = std::make_unique<Texture>();
    PBR2Tex->Name = "PBR2Tex";
    PBR2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Tex->Filename.c_str(),
        PBR2Tex->Resource, PBR2Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR2Tex->Name] = std::move(PBR2Tex);

    auto PBR2Norm = std::make_unique<Texture>();
    PBR2Norm->Name = "PBR2Norm";
    PBR2Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Norm->Filename.c_str(),
        PBR2Norm->Resource, PBR2Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR2Norm->Name] = std::move(PBR2Norm);

    auto PBR2AO = std::make_unique<Texture>();
    PBR2AO->Name = "PBR2AO";
    PBR2AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2AO->Filename.c_str(),
        PBR2AO->Resource, PBR2AO->UploadHeap), true);
    GeometryManager::mTextures[PBR2AO->Name] = std::move(PBR2AO);

    auto PBR2Metallic = std::make_unique<Texture>();
    PBR2Metallic->Name = "PBR2Metallic";
    PBR2Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Metallic->Filename.c_str(),
        PBR2Metallic->Resource, PBR2Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR2Metallic->Name] = std::move(PBR2Metallic);

    auto PBR2Roughness = std::make_unique<Texture>();
    PBR2Roughness->Name = "PBR2Roughness";
    PBR2Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR2Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR2Roughness->Filename.c_str(),
        PBR2Roughness->Resource, PBR2Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR2Roughness->Name] = std::move(PBR2Roughness);

    auto PBR3Tex = std::make_unique<Texture>();
    PBR3Tex->Name = "PBR3Tex";
    PBR3Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Tex->Filename.c_str(),
        PBR3Tex->Resource, PBR3Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR3Tex->Name] = std::move(PBR3Tex);

    auto PBR3Norm = std::make_unique<Texture>();
    PBR3Norm->Name = "PBR3Norm";
    PBR3Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Norm->Filename.c_str(),
        PBR3Norm->Resource, PBR3Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR3Norm->Name] = std::move(PBR3Norm);

    auto PBR3AO = std::make_unique<Texture>();
    PBR3AO->Name = "PBR3AO";
    PBR3AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3AO->Filename.c_str(),
        PBR3AO->Resource, PBR3AO->UploadHeap), true);
    GeometryManager::mTextures[PBR3AO->Name] = std::move(PBR3AO);

    auto PBR3Metallic = std::make_unique<Texture>();
    PBR3Metallic->Name = "PBR3Metallic";
    PBR3Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Metallic->Filename.c_str(),
        PBR3Metallic->Resource, PBR3Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR3Metallic->Name] = std::move(PBR3Metallic);

    auto PBR3Roughness = std::make_unique<Texture>();
    PBR3Roughness->Name = "PBR3Roughness";
    PBR3Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR3Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR3Roughness->Filename.c_str(),
        PBR3Roughness->Resource, PBR3Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR3Roughness->Name] = std::move(PBR3Roughness);

    auto PBR4Tex = std::make_unique<Texture>();
    PBR4Tex->Name = "PBR4Tex";
    PBR4Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Tex->Filename.c_str(),
        PBR4Tex->Resource, PBR4Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR4Tex->Name] = std::move(PBR4Tex);

    auto PBR4Norm = std::make_unique<Texture>();
    PBR4Norm->Name = "PBR4Norm";
    PBR4Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Norm->Filename.c_str(),
        PBR4Norm->Resource, PBR4Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR4Norm->Name] = std::move(PBR4Norm);

    auto PBR4AO = std::make_unique<Texture>();
    PBR4AO->Name = "PBR4AO";
    PBR4AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4AO->Filename.c_str(),
        PBR4AO->Resource, PBR4AO->UploadHeap), true);
    GeometryManager::mTextures[PBR4AO->Name] = std::move(PBR4AO);

    auto PBR4Metallic = std::make_unique<Texture>();
    PBR4Metallic->Name = "PBR4Metallic";
    PBR4Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Metallic->Filename.c_str(),
        PBR4Metallic->Resource, PBR4Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR4Metallic->Name] = std::move(PBR4Metallic);

    auto PBR4Roughness = std::make_unique<Texture>();
    PBR4Roughness->Name = "PBR4Roughness";
    PBR4Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR4Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR4Roughness->Filename.c_str(),
        PBR4Roughness->Resource, PBR4Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR4Roughness->Name] = std::move(PBR4Roughness);

    auto PBR5Tex = std::make_unique<Texture>();
    PBR5Tex->Name = "PBR5Tex";
    PBR5Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Tex->Filename.c_str(),
        PBR5Tex->Resource, PBR5Tex->UploadHeap), true);
    GeometryManager::mTextures[PBR5Tex->Name] = std::move(PBR5Tex);

    auto PBR5Norm = std::make_unique<Texture>();
    PBR5Norm->Name = "PBR5Norm";
    PBR5Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Norm.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Norm->Filename.c_str(),
        PBR5Norm->Resource, PBR5Norm->UploadHeap), true);
    GeometryManager::mTextures[PBR5Norm->Name] = std::move(PBR5Norm);

    auto PBR5AO = std::make_unique<Texture>();
    PBR5AO->Name = "PBR5AO";
    PBR5AO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5AO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5AO->Filename.c_str(),
        PBR5AO->Resource, PBR5AO->UploadHeap), true);
    GeometryManager::mTextures[PBR5AO->Name] = std::move(PBR5AO);

    auto PBR5Metallic = std::make_unique<Texture>();
    PBR5Metallic->Name = "PBR5Metallic";
    PBR5Metallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Metallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Metallic->Filename.c_str(),
        PBR5Metallic->Resource, PBR5Metallic->UploadHeap), true);
    GeometryManager::mTextures[PBR5Metallic->Name] = std::move(PBR5Metallic);

    auto PBR5Roughness = std::make_unique<Texture>();
    PBR5Roughness->Name = "PBR5Roughness";
    PBR5Roughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PBR5Roughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PBR5Roughness->Filename.c_str(),
        PBR5Roughness->Resource, PBR5Roughness->UploadHeap), true);
    GeometryManager::mTextures[PBR5Roughness->Name] = std::move(PBR5Roughness);

    auto IrradianceMap = std::make_unique<Texture>();
    IrradianceMap->Name = "IrradianceMap";
    IrradianceMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\IrradianceMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), IrradianceMap->Filename.c_str(),
        IrradianceMap->Resource, IrradianceMap->UploadHeap), true);
    GeometryManager::mTextures[IrradianceMap->Name] = std::move(IrradianceMap);

    auto PreFilteredEnvMap = std::make_unique<Texture>();
    PreFilteredEnvMap->Name = "PreFilteredEnvMap";
    PreFilteredEnvMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\PreFilteredEnvMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), PreFilteredEnvMap->Filename.c_str(),
        PreFilteredEnvMap->Resource, PreFilteredEnvMap->UploadHeap), true);
    GeometryManager::mTextures[PreFilteredEnvMap->Name] = std::move(PreFilteredEnvMap);

    auto IntegrationMap = std::make_unique<Texture>();
    IntegrationMap->Name = "IntegrationMap";
    IntegrationMap->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\IntegrationMap.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), IntegrationMap->Filename.c_str(),
        IntegrationMap->Resource, IntegrationMap->UploadHeap), true);
    GeometryManager::mTextures[IntegrationMap->Name] = std::move(IntegrationMap);

    auto TerrainTex = std::make_unique<Texture>();
    TerrainTex->Name = "TerrainTex";
    TerrainTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\TerrainTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), TerrainTex->Filename.c_str(),
        TerrainTex->Resource, TerrainTex->UploadHeap), true);
    GeometryManager::mTextures[TerrainTex->Name] = std::move(TerrainTex);

    auto TerrainHeight = std::make_unique<Texture>();
    TerrainHeight->Name = "TerrainHeight";
    TerrainHeight->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\TerrainHeight.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), TerrainHeight->Filename.c_str(),
        TerrainHeight->Resource, TerrainHeight->UploadHeap), true);
    GeometryManager::mTextures[TerrainHeight->Name] = std::move(TerrainHeight);

    auto RainTex = std::make_unique<Texture>();
    RainTex->Name = "RainTex";
    RainTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\RainTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), RainTex->Filename.c_str(),
        RainTex->Resource, RainTex->UploadHeap), true);
    GeometryManager::mTextures[RainTex->Name] = std::move(RainTex);

    auto RainCircleTex = std::make_unique<Texture>();
    RainCircleTex->Name = "RainCircleTex";
    RainCircleTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\RainCircleTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), RainCircleTex->Filename.c_str(),
        RainCircleTex->Resource, RainCircleTex->UploadHeap), true);
    GeometryManager::mTextures[RainCircleTex->Name] = std::move(RainCircleTex);

    auto Terrain2Tex = std::make_unique<Texture>();
    Terrain2Tex->Name = "Terrain2Tex";
    Terrain2Tex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Tex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Tex->Filename.c_str(),
        Terrain2Tex->Resource, Terrain2Tex->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Tex->Name] = std::move(Terrain2Tex);

    auto Terrain2Norm = std::make_unique<Texture>();
    Terrain2Norm->Name = "Terrain2Norm";
    Terrain2Norm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Normal.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Norm->Filename.c_str(),
        Terrain2Norm->Resource, Terrain2Norm->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Norm->Name] = std::move(Terrain2Norm);

    auto Terrain2Height = std::make_unique<Texture>();
    Terrain2Height->Name = "Terrain2Height";
    Terrain2Height->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\Terrain2Height.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), Terrain2Height->Filename.c_str(),
        Terrain2Height->Resource, Terrain2Height->UploadHeap), true);
    GeometryManager::mTextures[Terrain2Height->Name] = std::move(Terrain2Height);

    auto HamburgerTex = std::make_unique<Texture>();
    HamburgerTex->Name = "HamburgerTex";
    HamburgerTex->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerTex.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerTex->Filename.c_str(),
        HamburgerTex->Resource, HamburgerTex->UploadHeap), true);
    GeometryManager::mTextures[HamburgerTex->Name] = std::move(HamburgerTex);

    auto HamburgerNorm = std::make_unique<Texture>();
    HamburgerNorm->Name = "HamburgerNorm";
    HamburgerNorm->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerNormal.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerNorm->Filename.c_str(),
        HamburgerNorm->Resource, HamburgerNorm->UploadHeap), true);
    GeometryManager::mTextures[HamburgerNorm->Name] = std::move(HamburgerNorm);

    auto HamburgerAO = std::make_unique<Texture>();
    HamburgerAO->Name = "HamburgerAO";
    HamburgerAO->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerAO.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerAO->Filename.c_str(),
        HamburgerAO->Resource, HamburgerAO->UploadHeap), true);
    GeometryManager::mTextures[HamburgerAO->Name] = std::move(HamburgerAO);

    auto HamburgerMetallic = std::make_unique<Texture>();
    HamburgerMetallic->Name = "HamburgerMetallic";
    HamburgerMetallic->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerMetallic.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerMetallic->Filename.c_str(),
        HamburgerMetallic->Resource, HamburgerMetallic->UploadHeap), true);
    GeometryManager::mTextures[HamburgerMetallic->Name] = std::move(HamburgerMetallic);

    auto HamburgerRoughness = std::make_unique<Texture>();
    HamburgerRoughness->Name = "HamburgerRoughness";
    HamburgerRoughness->Filename = L"C:\\Users\\MSI SWORD 15\\source\\repos\\GraphicEngineDirectX12\\GraphicEngine\\Textures\\HamburgerRoughness.dds";
    ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
        mCommandList.Get(), HamburgerRoughness->Filename.c_str(),
        HamburgerRoughness->Resource, HamburgerRoughness->UploadHeap), true);
    GeometryManager::mTextures[HamburgerRoughness->Name] = std::move(HamburgerRoughness);
}

void GeometryManager::BuildSkyboxGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData skysphere = geoGen.CreateSphere(0.5f, 20, 20, 0.0f, 0.0f, 0.0f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry skyboxSubmesh;
    skyboxSubmesh.IndexCount = (UINT)skysphere.Indices32.size();
    skyboxSubmesh.StartIndexLocation = totalIndexCount;
    skyboxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += skyboxSubmesh.IndexCount;
    totalVertexCount += skysphere.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < skysphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = skysphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = skysphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = skysphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = skysphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += skysphere.Vertices.size();

    indices.insert(indices.end(), std::begin(skysphere.GetIndices16()), std::end(skysphere.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "skyboxGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["skybox"] = skyboxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildShadowMaps(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    mShadowMap256 = std::make_unique<ShadowMap>(device.Get(), 256, 256);
    mShadowMap512 = std::make_unique<ShadowMap>(device.Get(), 512, 512);
    mShadowMap1024 = std::make_unique<ShadowMap>(device.Get(), 1024, 1024);
    mShadowMap2048 = std::make_unique<ShadowMap>(device.Get(), 2048, 2048);

    mShadowMapScene6 = std::make_unique<ShadowMap>(device.Get(), 2048, 2048);
}

void GeometryManager::BuildShapeGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData box = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box2 = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 1, 1);
    GeometryGenerator::MeshData box3 = geoGen.CreateBoxTiling(5.f, 5.f, 5.f, 5, 1);
    GeometryGenerator::MeshData box4 = geoGen.CreateBoxTiling(1.f, 1.f, 1.f, 1, 1);
    GeometryGenerator::MeshData boxFPS = geoGen.CreateBoxTiling(2.f, 2.f, 2.f, 5, 1);
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.0f, 7, 7, 0.f, 0.f, 0.f);
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(200.0f, 200.0f, 100, 100);
    GeometryGenerator::MeshData cone = geoGen.CreateCone(1.0f, 0.2f, 20);
    GeometryGenerator::MeshData skyBox = geoGen.CreateBackSphere(1.f, 50.f, 50.f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry boxSubmesh;
    boxSubmesh.IndexCount = (UINT)box.Indices32.size();
    boxSubmesh.StartIndexLocation = totalIndexCount;
    boxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh.IndexCount;
    totalVertexCount += box.Vertices.size();

    SubmeshGeometry boxSubmesh2;
    boxSubmesh2.IndexCount = (UINT)box2.Indices32.size();
    boxSubmesh2.StartIndexLocation = totalIndexCount;
    boxSubmesh2.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh2.IndexCount;
    totalVertexCount += box2.Vertices.size();

    SubmeshGeometry boxSubmesh3;
    boxSubmesh3.IndexCount = (UINT)box3.Indices32.size();
    boxSubmesh3.StartIndexLocation = totalIndexCount;
    boxSubmesh3.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh3.IndexCount;
    totalVertexCount += box3.Vertices.size();

    SubmeshGeometry boxSubmesh4;
    boxSubmesh4.IndexCount = (UINT)box4.Indices32.size();
    boxSubmesh4.StartIndexLocation = totalIndexCount;
    boxSubmesh4.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh4.IndexCount;
    totalVertexCount += box4.Vertices.size();

    SubmeshGeometry boxSubmeshFPS;
    boxSubmeshFPS.IndexCount = (UINT)boxFPS.Indices32.size();
    boxSubmeshFPS.StartIndexLocation = totalIndexCount;
    boxSubmeshFPS.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmeshFPS.IndexCount;
    totalVertexCount += boxFPS.Vertices.size();

    SubmeshGeometry pointLight1Submesh;
    pointLight1Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight1Submesh.StartIndexLocation = totalIndexCount;
    pointLight1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight1Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight2Submesh;
    pointLight2Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight2Submesh.StartIndexLocation = totalIndexCount;
    pointLight2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight2Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight3Submesh;
    pointLight3Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight3Submesh.StartIndexLocation = totalIndexCount;
    pointLight3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight3Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry gridSubmesh;
    gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
    gridSubmesh.StartIndexLocation = totalIndexCount;
    gridSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += gridSubmesh.IndexCount;
    totalVertexCount += grid.Vertices.size();

    SubmeshGeometry coneSubmesh;
    coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
    coneSubmesh.StartIndexLocation = totalIndexCount;
    coneSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += coneSubmesh.IndexCount;
    totalVertexCount += cone.Vertices.size();

    SubmeshGeometry skyboxSubmesh;
    skyboxSubmesh.IndexCount = (UINT)skyBox.Indices32.size();
    skyboxSubmesh.StartIndexLocation = totalIndexCount;
    skyboxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += skyboxSubmesh.IndexCount;
    totalVertexCount += skyBox.Vertices.size();

    std::vector<VertexLightStage> verticesLightStage =
    {
        { DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
        { DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
        { DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
        { DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }
    };

    const UINT vbByteSizeLight = (UINT)verticesLightStage.size() * sizeof(VertexLightStage);

    auto screenQuad = std::make_unique<MeshGeometry>();
    screenQuad->Name = "screenQuad";

    ThrowIfFailed(D3DCreateBlob(vbByteSizeLight, &screenQuad->VertexBufferCPU));
    CopyMemory(screenQuad->VertexBufferCPU->GetBufferPointer(), verticesLightStage.data(), vbByteSizeLight);

    screenQuad->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), verticesLightStage.data(), vbByteSizeLight, screenQuad->VertexBufferUploader);

    screenQuad->VertexByteStride = sizeof(VertexLightStage);
    screenQuad->VertexBufferByteSize = vbByteSizeLight;

    GeometryManager::mGeometries[screenQuad->Name] = std::move(screenQuad);

    std::vector<Vertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;
    // Full screen quad

    for (size_t i = 0; i < box.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box.Vertices[i].TangentU;
    }
    totalVertexCount2 += box.Vertices.size();
    for (size_t i = 0; i < box2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box2.Vertices[i].TangentU;
    }
    totalVertexCount2 += box2.Vertices.size();
    for (size_t i = 0; i < box3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box3.Vertices[i].TangentU;
    }
    totalVertexCount2 += box3.Vertices.size();
    for (size_t i = 0; i < box4.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = box4.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = box4.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = box4.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = box4.Vertices[i].TangentU;
    }
    totalVertexCount2 += box4.Vertices.size();
    for (size_t i = 0; i < boxFPS.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxFPS.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxFPS.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxFPS.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxFPS.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxFPS.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphere.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphere.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphere.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = grid.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = grid.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = grid.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = grid.Vertices[i].TangentU;
    }
    totalVertexCount2 += grid.Vertices.size();
    for (size_t i = 0; i < cone.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = cone.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = cone.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = cone.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = cone.Vertices[i].TangentU;
    }
    totalVertexCount2 += cone.Vertices.size();
    for (size_t i = 0; i < skyBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = skyBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = skyBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = skyBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = skyBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += skyBox.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
    indices.insert(indices.end(), std::begin(box2.GetIndices16()), std::end(box2.GetIndices16()));
    indices.insert(indices.end(), std::begin(box3.GetIndices16()), std::end(box3.GetIndices16()));
    indices.insert(indices.end(), std::begin(box4.GetIndices16()), std::end(box4.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxFPS.GetIndices16()), std::end(boxFPS.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
    indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));
    indices.insert(indices.end(), std::begin(skyBox.GetIndices16()), std::end(skyBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "boxGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["box"] = boxSubmesh;
    geo->DrawArgs["box2"] = boxSubmesh2;
    geo->DrawArgs["box3"] = boxSubmesh3;
    geo->DrawArgs["box4"] = boxSubmesh4;
    geo->DrawArgs["boxFPS"] = boxSubmeshFPS;
    geo->DrawArgs["pointLight1"] = pointLight1Submesh;
    geo->DrawArgs["pointLight2"] = pointLight2Submesh;
    geo->DrawArgs["pointLight3"] = pointLight3Submesh;
    geo->DrawArgs["grid"] = gridSubmesh;
    geo->DrawArgs["cone"] = coneSubmesh;
    geo->DrawArgs["skybox"] = skyboxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene3Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData instancingBox = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry boxSubmesh;
    boxSubmesh.IndexCount = (UINT)instancingBox.Indices32.size();
    boxSubmesh.StartIndexLocation = totalIndexCount;
    boxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += boxSubmesh.IndexCount;
    totalVertexCount += instancingBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < instancingBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = instancingBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = instancingBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = instancingBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = instancingBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += instancingBox.Vertices.size();

    DirectX::BoundingBox bounds;
    bounds.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    bounds.Extents = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

    indices.insert(indices.end(), std::begin(instancingBox.GetIndices16()), std::end(instancingBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene3Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;
    boxSubmesh.Bounds = bounds;

    geo->DrawArgs["boxInstancing"] = boxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);

    instanceDataScene3 = std::vector<DirectX::XMMATRIX>(729);
    for (int i = 0; i < 729; ++i)
    {
        int x = i % 27;
        int y = i / 27;
        int index = 0;
        if (y >= x)
            index = y * y + x;
        else index = (x + 1) * (x + 1) - y - 1;

        instanceDataScene3[index] = DirectX::XMMatrixTranslation((float)x * 2, 0, (float)y * 2);
    }
}

void GeometryManager::BuildScene3LODGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereLOD3 = geoGen.CreateSphere(3.0f, 20, 20, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD2 = geoGen.CreateSphere(3.0f, 12, 12, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD1 = geoGen.CreateSphere(3.0f, 8, 8, 0, 0, 0);
    GeometryGenerator::MeshData sphereLOD0 = geoGen.CreateSphere(3.0f, 5, 5, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereLOD3Submesh;
    sphereLOD3Submesh.IndexCount = (UINT)sphereLOD3.Indices32.size();
    sphereLOD3Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD3Submesh.IndexCount;
    totalVertexCount += sphereLOD3.Vertices.size();

    SubmeshGeometry sphereLOD2Submesh;
    sphereLOD2Submesh.IndexCount = (UINT)sphereLOD2.Indices32.size();
    sphereLOD2Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD2Submesh.IndexCount;
    totalVertexCount += sphereLOD2.Vertices.size();

    SubmeshGeometry sphereLOD1Submesh;
    sphereLOD1Submesh.IndexCount = (UINT)sphereLOD1.Indices32.size();
    sphereLOD1Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD1Submesh.IndexCount;
    totalVertexCount += sphereLOD1.Vertices.size();

    SubmeshGeometry sphereLOD0Submesh;
    sphereLOD0Submesh.IndexCount = (UINT)sphereLOD0.Indices32.size();
    sphereLOD0Submesh.StartIndexLocation = totalIndexCount;
    sphereLOD0Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereLOD0Submesh.IndexCount;
    totalVertexCount += sphereLOD0.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereLOD3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD3.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD3.Vertices.size();
    for (size_t i = 0; i < sphereLOD2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD2.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD2.Vertices.size();
    for (size_t i = 0; i < sphereLOD1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD1.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD1.Vertices.size();
    for (size_t i = 0; i < sphereLOD0.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereLOD0.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereLOD0.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereLOD0.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereLOD0.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereLOD0.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereLOD3.GetIndices16()), std::end(sphereLOD3.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD2.GetIndices16()), std::end(sphereLOD2.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD1.GetIndices16()), std::end(sphereLOD1.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereLOD0.GetIndices16()), std::end(sphereLOD0.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene3LODGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphereLOD3"] = sphereLOD3Submesh;
    geo->DrawArgs["sphereLOD2"] = sphereLOD2Submesh;
    geo->DrawArgs["sphereLOD1"] = sphereLOD1Submesh;
    geo->DrawArgs["sphereLOD0"] = sphereLOD0Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene4Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData animateBox = geoGen.CreateBox(2.0f, 2.0f, 2.0f, 0);
    GeometryGenerator::MeshData tilingBox = geoGen.CreateBoxTiling(2.0f, 2.0f, 2.0f, 0, 1);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry animateBoxSubmesh;
    animateBoxSubmesh.IndexCount = (UINT)animateBox.Indices32.size();
    animateBoxSubmesh.StartIndexLocation = totalIndexCount;
    animateBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += animateBoxSubmesh.IndexCount;
    totalVertexCount += animateBox.Vertices.size();

    SubmeshGeometry tilingBoxSubmesh;
    tilingBoxSubmesh.IndexCount = (UINT)tilingBox.Indices32.size();
    tilingBoxSubmesh.StartIndexLocation = totalIndexCount;
    tilingBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += tilingBoxSubmesh.IndexCount;
    totalVertexCount += tilingBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < animateBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = animateBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = animateBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = animateBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = animateBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += animateBox.Vertices.size();
    for (size_t i = 0; i < tilingBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = tilingBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = tilingBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = tilingBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = tilingBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += tilingBox.Vertices.size();

    indices.insert(indices.end(), std::begin(animateBox.GetIndices16()), std::end(animateBox.GetIndices16()));
    indices.insert(indices.end(), std::begin(tilingBox.GetIndices16()), std::end(tilingBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene4Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["animateBox"] = animateBoxSubmesh;
    geo->DrawArgs["tilingBox"] = tilingBoxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene5Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData tessellationBox = geoGen.CreateBox(5.0f, 5.0f, 5.0f, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry tessellationBoxSubmesh;
    tessellationBoxSubmesh.IndexCount = (UINT)tessellationBox.Indices32.size();
    tessellationBoxSubmesh.StartIndexLocation = totalIndexCount;
    tessellationBoxSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += tessellationBoxSubmesh.IndexCount;
    totalVertexCount += tessellationBox.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < tessellationBox.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = tessellationBox.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = tessellationBox.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = tessellationBox.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = tessellationBox.Vertices[i].TangentU;
    }
    totalVertexCount2 += tessellationBox.Vertices.size();

    indices.insert(indices.end(), std::begin(tessellationBox.GetIndices16()), std::end(tessellationBox.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene5Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["tessellationBox"] = tessellationBoxSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene6Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData grassPlatform = geoGen.CreateBoxTiling(100.0f, 0.1f, 100.0f, 0, 10);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry grassPlatformSubmesh;
    grassPlatformSubmesh.IndexCount = (UINT)grassPlatform.Indices32.size();
    grassPlatformSubmesh.StartIndexLocation = totalIndexCount;
    grassPlatformSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += grassPlatformSubmesh.IndexCount;
    totalVertexCount += grassPlatform.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < grassPlatform.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = grassPlatform.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = grassPlatform.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = grassPlatform.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = grassPlatform.Vertices[i].TangentU;
    }
    totalVertexCount2 += grassPlatform.Vertices.size();

    indices.insert(indices.end(), std::begin(grassPlatform.GetIndices16()), std::end(grassPlatform.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene6GrassPlatform";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["GrassPlatform"] = grassPlatformSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene7Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData boxMesh1 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);
    GeometryGenerator::MeshData boxMesh2 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);
    GeometryGenerator::MeshData boxMesh3 = geoGen.CreateBoxTiling(2.2f, 2.2f, 2.2f, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry box1Submesh;
    box1Submesh.IndexCount = (UINT)boxMesh1.Indices32.size();
    box1Submesh.StartIndexLocation = totalIndexCount;
    box1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box1Submesh.IndexCount;
    totalVertexCount += boxMesh1.Vertices.size();
    SubmeshGeometry box2Submesh;
    box2Submesh.IndexCount = (UINT)boxMesh2.Indices32.size();
    box2Submesh.StartIndexLocation = totalIndexCount;
    box2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box2Submesh.IndexCount;
    totalVertexCount += boxMesh2.Vertices.size();
    SubmeshGeometry box3Submesh;
    box3Submesh.IndexCount = (UINT)boxMesh3.Indices32.size();
    box3Submesh.StartIndexLocation = totalIndexCount;
    box3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += box3Submesh.IndexCount;
    totalVertexCount += boxMesh3.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < boxMesh1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh1.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh1.Vertices.size();
    for (size_t i = 0; i < boxMesh2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh2.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh2.Vertices.size();
    for (size_t i = 0; i < boxMesh3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = boxMesh3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = boxMesh3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = boxMesh3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = boxMesh3.Vertices[i].TangentU;
    }
    totalVertexCount2 += boxMesh3.Vertices.size();

    indices.insert(indices.end(), std::begin(boxMesh1.GetIndices16()), std::end(boxMesh1.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxMesh2.GetIndices16()), std::end(boxMesh2.GetIndices16()));
    indices.insert(indices.end(), std::begin(boxMesh3.GetIndices16()), std::end(boxMesh3.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene7Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["box1"] = box1Submesh;
    geo->DrawArgs["box2"] = box2Submesh;
    geo->DrawArgs["box3"] = box3Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene9Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh1 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh2 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh3 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh4 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);
    GeometryGenerator::MeshData sphereMesh5 = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphere1Submesh;
    sphere1Submesh.IndexCount = (UINT)sphereMesh1.Indices32.size();
    sphere1Submesh.StartIndexLocation = totalIndexCount;
    sphere1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere1Submesh.IndexCount;
    totalVertexCount += sphereMesh1.Vertices.size();
    SubmeshGeometry sphere2Submesh;
    sphere2Submesh.IndexCount = (UINT)sphereMesh2.Indices32.size();
    sphere2Submesh.StartIndexLocation = totalIndexCount;
    sphere2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere2Submesh.IndexCount;
    totalVertexCount += sphereMesh2.Vertices.size();
    SubmeshGeometry sphere3Submesh;
    sphere3Submesh.IndexCount = (UINT)sphereMesh3.Indices32.size();
    sphere3Submesh.StartIndexLocation = totalIndexCount;
    sphere3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere3Submesh.IndexCount;
    totalVertexCount += sphereMesh3.Vertices.size();
    SubmeshGeometry sphere4Submesh;
    sphere4Submesh.IndexCount = (UINT)sphereMesh4.Indices32.size();
    sphere4Submesh.StartIndexLocation = totalIndexCount;
    sphere4Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere4Submesh.IndexCount;
    totalVertexCount += sphereMesh4.Vertices.size();
    SubmeshGeometry sphere5Submesh;
    sphere5Submesh.IndexCount = (UINT)sphereMesh5.Indices32.size();
    sphere5Submesh.StartIndexLocation = totalIndexCount;
    sphere5Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphere5Submesh.IndexCount;
    totalVertexCount += sphereMesh5.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh1.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh1.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh1.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh1.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh1.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh1.Vertices.size();
    for (size_t i = 0; i < sphereMesh2.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh2.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh2.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh2.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh2.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh2.Vertices.size();
    for (size_t i = 0; i < sphereMesh3.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh3.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh3.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh3.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh3.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh3.Vertices.size();
    for (size_t i = 0; i < sphereMesh4.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh4.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh4.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh4.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh4.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh4.Vertices.size();
    for (size_t i = 0; i < sphereMesh5.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh5.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh5.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh5.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh5.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh5.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh1.GetIndices16()), std::end(sphereMesh1.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh2.GetIndices16()), std::end(sphereMesh2.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh3.GetIndices16()), std::end(sphereMesh3.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh4.GetIndices16()), std::end(sphereMesh4.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphereMesh5.GetIndices16()), std::end(sphereMesh5.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene9Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere1"] = sphere1Submesh;
    geo->DrawArgs["sphere2"] = sphere2Submesh;
    geo->DrawArgs["sphere3"] = sphere3Submesh;
    geo->DrawArgs["sphere4"] = sphere4Submesh;
    geo->DrawArgs["sphere5"] = sphere5Submesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene9RMDemoGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh = geoGen.CreateSphere(2.2f, 30, 30, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereSubmesh;
    sphereSubmesh.IndexCount = (UINT)sphereMesh.Indices32.size();
    sphereSubmesh.StartIndexLocation = totalIndexCount;
    sphereSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereSubmesh.IndexCount;
    totalVertexCount += sphereMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh.GetIndices16()), std::end(sphereMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene9RMDemoGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere"] = sphereSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildModelsGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    ModelLoader::LoadModelML("Obj/bonfire.obj", vertices, indices);
    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].Pos = DirectX::XMFLOAT3(vertices[i].Pos.x / 10 + 10.8f, vertices[i].Pos.y / 10, vertices[i].Pos.z / 10 + 1.f);
    }

    SubmeshGeometry bonfireSubmesh;
    bonfireSubmesh.IndexCount = indices.size();
    bonfireSubmesh.StartIndexLocation = 0;
    bonfireSubmesh.BaseVertexLocation = 0;
    bonfireSubmesh.VertexCount = vertices.size();

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene6Bonfire";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["Bonfire"] = bonfireSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene10DebugGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.0f, 7, 7, 0.f, 0.f, 0.f);
    GeometryGenerator::MeshData cone = geoGen.CreateCone(1.0f, 0.2f, 20);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry pointLight1Submesh;
    pointLight1Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight1Submesh.StartIndexLocation = totalIndexCount;
    pointLight1Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight1Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight2Submesh;
    pointLight2Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight2Submesh.StartIndexLocation = totalIndexCount;
    pointLight2Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight2Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry pointLight3Submesh;
    pointLight3Submesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLight3Submesh.StartIndexLocation = totalIndexCount;
    pointLight3Submesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLight3Submesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();

    SubmeshGeometry coneSubmesh;
    coneSubmesh.IndexCount = (UINT)cone.Indices32.size();
    coneSubmesh.StartIndexLocation = totalIndexCount;
    coneSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += coneSubmesh.IndexCount;
    totalVertexCount += cone.Vertices.size();

    std::vector<DebugVertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;

    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();
    for (size_t i = 0; i < cone.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = cone.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += cone.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
    indices.insert(indices.end(), std::begin(cone.GetIndices16()), std::end(cone.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "debugGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["pointLight1"] = pointLight1Submesh;
    geo->DrawArgs["pointLight2"] = pointLight2Submesh;
    geo->DrawArgs["pointLight3"] = pointLight3Submesh;
    geo->DrawArgs["cone"] = coneSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene10MoreLightGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData sphere = geoGen.CreateSphere(1.f, 7, 7, 0.f, 0.f, 0.f);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry pointLightSubmesh;
    pointLightSubmesh.IndexCount = (UINT)sphere.Indices32.size();
    pointLightSubmesh.StartIndexLocation = totalIndexCount;
    pointLightSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += pointLightSubmesh.IndexCount;
    totalVertexCount += sphere.Vertices.size();


    std::vector<DebugVertex> vertices(totalVertexCount);
    UINT totalVertexCount2 = 0;

    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Position = sphere.Vertices[i].Position;
        vertices[i + totalVertexCount2].Color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    totalVertexCount2 += sphere.Vertices.size();


    std::vector<std::uint16_t> indices;
    indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "moreLightGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["pointLight"] = pointLightSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene12Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData planeMesh = geoGen.CreateGrid(100, 100, 50, 50);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry planeSubmesh;
    planeSubmesh.IndexCount = (UINT)planeMesh.Indices32.size();
    planeSubmesh.StartIndexLocation = totalIndexCount;
    planeSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += planeSubmesh.IndexCount;
    totalVertexCount += planeMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < planeMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = planeMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = planeMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = planeMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = planeMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += planeMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(planeMesh.GetIndices16()), std::end(planeMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene12Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["plane"] = planeSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene13Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    GeometryGenerator geoGen;

    GeometryGenerator::MeshData sphereMesh = geoGen.CreateSphere(1.0f, 250, 250, 0, 0, 0);

    UINT totalIndexCount = 0;
    UINT totalVertexCount = 0;

    SubmeshGeometry sphereSubmesh;
    sphereSubmesh.IndexCount = (UINT)sphereMesh.Indices32.size();
    sphereSubmesh.StartIndexLocation = totalIndexCount;
    sphereSubmesh.BaseVertexLocation = totalVertexCount;
    totalIndexCount += sphereSubmesh.IndexCount;
    totalVertexCount += sphereMesh.Vertices.size();

    std::vector<Vertex> vertices(totalVertexCount);
    std::vector<std::uint16_t> indices;

    UINT totalVertexCount2 = 0;
    for (size_t i = 0; i < sphereMesh.Vertices.size(); ++i)
    {
        vertices[i + totalVertexCount2].Pos = sphereMesh.Vertices[i].Position;
        vertices[i + totalVertexCount2].Normal = sphereMesh.Vertices[i].Normal;
        vertices[i + totalVertexCount2].TexC = sphereMesh.Vertices[i].TexC;
        vertices[i + totalVertexCount2].TangentU = sphereMesh.Vertices[i].TangentU;
    }
    totalVertexCount2 += sphereMesh.Vertices.size();

    indices.insert(indices.end(), std::begin(sphereMesh.GetIndices16()), std::end(sphereMesh.GetIndices16()));

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene13Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["sphere"] = sphereSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);

    BuildScene13InstanceBuffer(commandList, device);
    BuildScene13OctreeGeometry(commandList, device);
}

void GeometryManager::BuildScene13InstanceBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    int instanceCount = 150;
    std::vector<InstanceDataGameObject> instanceData(instanceCount);
    for (UINT i = 0; i < instanceCount; ++i)
    {
        float xTranslation = std::rand() % 40 - 20;
        float yTranslation = std::rand() % 40 - 20;
        float zTranslation = std::rand() % 40 - 20;

        DirectX::XMStoreFloat4x4(&instanceData[i].WorldMatrix,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(xTranslation, yTranslation, zTranslation)));
        instanceData[i].Color = DirectX::XMFLOAT4(sin(i), cos(i), sin(2 * i), 1.0f);

        GameObject* gameObject = new GameObject();
        DirectX::BoundingBox boundingBox;
        boundingBox.Center = DirectX::XMFLOAT3(xTranslation, yTranslation, zTranslation);
        boundingBox.Extents = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
        gameObject->boundingBox = boundingBox;
        gameObject->objectID = i;
        gameObjects.push_back(gameObject);
    }
    instancesDataOcTree = instanceData;

    const UINT instanceBufferSize = instanceCount * sizeof(InstanceDataGameObject);

    D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);

    device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&instanceOcTreeBuffer));

    D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&instanceOcTreeUploadBuffer));

    D3D12_SUBRESOURCE_DATA instanceDataSub =
    {
        .pData = instanceData.data(),
        .RowPitch = instanceBufferSize,
        .SlicePitch = instanceBufferSize
    };

    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceOcTreeBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_COPY_DEST);
    commandList->ResourceBarrier(1, &barrier1);

    UpdateSubresources<1>(commandList.Get(),
        instanceOcTreeBuffer.Get(),
        instanceOcTreeUploadBuffer.Get(),
        0, 0, 1, &instanceDataSub);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(
        instanceOcTreeBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier2);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = 150;
    srvDesc.Buffer.StructureByteStride = sizeof(InstanceDataGameObject);
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(DescriptorHeapManager::mSrvHeap->GetCPUDescriptorHandleForHeapStart());
    hDescriptor.Offset(57, DescriptorHeapManager::mCbvSrvDescriptorSize);
    device->CreateShaderResourceView(
        instanceOcTreeBuffer.Get(),
        &srvDesc,
        hDescriptor);

    DirectX::BoundingBox worldBoundingBox;
    worldBoundingBox.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    worldBoundingBox.Extents = DirectX::XMFLOAT3(20.0f, 20.0f, 20.0f);
    octreeScene13 = new Octree(device.Get(), worldBoundingBox);
    octreeScene13->Build(gameObjects);
}

void GeometryManager::BuildScene13OctreeGeometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    std::vector<DebugVertexOcTree> vertices;
    std::vector<std::uint16_t> indices;

    octreeScene13->CollectDebugNodes(octreeScene13->GetRoot(), vertices, indices);

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(DebugVertexOcTree);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene13OctreeGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(DebugVertexOcTree);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R16_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry octreeSubmesh;
    octreeSubmesh.IndexCount = indices.size();
    octreeSubmesh.StartIndexLocation = 0;
    octreeSubmesh.BaseVertexLocation = 0;

    geo->DrawArgs["octree"] = octreeSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildScene14Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
}

void GeometryManager::BuildScene15Geometry(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    ModelLoader::LoadModelML("Obj/hamburger.obj", vertices, indices);
    for (int i = 0; i < vertices.size(); ++i)
    {
        vertices[i].Pos = DirectX::XMFLOAT3(vertices[i].Pos.x * 10.f, vertices[i].Pos.y * 10.f, vertices[i].Pos.z * 10.f);
    }

    SubmeshGeometry hamburgerSubmesh;
    hamburgerSubmesh.IndexCount = indices.size();
    hamburgerSubmesh.StartIndexLocation = 0;
    hamburgerSubmesh.BaseVertexLocation = 0;
    hamburgerSubmesh.VertexCount = vertices.size();

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

    auto geo = std::make_unique<MeshGeometry>();
    geo->Name = "scene15Geo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

    geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

    geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device.Get(),
        commandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

    geo->VertexByteStride = sizeof(Vertex);
    geo->VertexBufferByteSize = vbByteSize;
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = ibByteSize;

    geo->DrawArgs["Hamburger"] = hamburgerSubmesh;

    GeometryManager::mGeometries[geo->Name] = std::move(geo);
}

void GeometryManager::BuildQuadTreeTerrain(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    m_terrainQuadTree.Initialize(device.Get(), 6, GeometryManager::mGeometries, commandList);
}

void GeometryManager::BuildSponzaGeometryAndTextures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    LoadModel("Sponza/sponza.obj", Sponza, device.Get(),
        GeometryManager::mGeometries, commandList, DescriptorHeapManager::mSponzaSrvHeap, DescriptorHeapManager::mCbvSrvDescriptorSize,
        sponzaTextures, sponzaTexturesUpload);
}

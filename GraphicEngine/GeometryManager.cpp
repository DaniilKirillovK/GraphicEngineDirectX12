#include "GeometryManager.h"

std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> GeometryManager::mGeometries = std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>();
std::unordered_map<std::string, std::unique_ptr<Material>> GeometryManager::mMaterials = std::unordered_map<std::string, std::unique_ptr<Material>>();
std::unordered_map<std::string, std::unique_ptr<Texture>> GeometryManager::mTextures = std::unordered_map<std::string, std::unique_ptr<Texture>>();
std::vector<std::unique_ptr<RenderItem>> GeometryManager::mAllRitems = std::vector<std::unique_ptr<RenderItem>>();
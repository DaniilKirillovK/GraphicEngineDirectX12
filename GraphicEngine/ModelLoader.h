#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <DirectXMath.h>
#include <vector>
#include "FrameResource.h"

class ModelLoader
{
public:
    // OBJ
    static bool LoadModelML(const std::string& filePath,
        std::vector<Vertex>& vertices,
        std::vector<uint32_t>& indices);

    static void ProcessNodeML(aiNode* node, const aiScene* scene,
        std::vector<Vertex>& vertices,
        std::vector<uint32_t>& indices,
        int& totalIndexes);

    static void ProcessMeshML(aiMesh* mesh, const aiScene* scene,
        std::vector<Vertex>& vertices,
        std::vector<uint32_t>& indices,
        int& totalIndexes);
};



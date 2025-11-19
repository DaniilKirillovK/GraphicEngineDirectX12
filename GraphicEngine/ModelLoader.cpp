#include "ModelLoader.h"
#include "filesystem"

bool ModelLoader::LoadModelML(const std::string& filePath,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_Triangulate |
        aiProcess_ConvertToLeftHanded |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace);

    int totalIndexes = 0;
    ProcessNodeML(scene->mRootNode, scene, vertices, indices, totalIndexes);
    return true;
}

void ModelLoader::ProcessNodeML(aiNode* node, const aiScene* scene,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    int& totalIndexes)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMeshML(mesh, scene, vertices, indices, totalIndexes);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNodeML(node->mChildren[i], scene, vertices, indices, totalIndexes);
    }
}

void ModelLoader::ProcessMeshML(aiMesh* mesh, const aiScene* scene,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    int& totalIndexes)
{
    // Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Position
        vertex.Pos.x = mesh->mVertices[i].x / 50;
        vertex.Pos.y = mesh->mVertices[i].y / 50;
        vertex.Pos.z = mesh->mVertices[i].z / 50;

        // Normal
        if (mesh->HasNormals())
        {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }

        // TexCoords
        if (mesh->mTextureCoords[0])
        {
            vertex.TexC.x = mesh->mTextureCoords[0][i].x;
            vertex.TexC.y = mesh->mTextureCoords[0][i].y;
        }
        else
        {
            vertex.TexC = { 0.0f, 0.0f };
        }

        // Tangent
        if (mesh->HasTangentsAndBitangents())
        {
            vertex.TangentU.x = mesh->mTangents[i].x;
            vertex.TangentU.y = mesh->mTangents[i].y;
            vertex.TangentU.z = mesh->mTangents[i].z;
        }

        vertices.push_back(vertex);
    }

    // Indexes
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j] + totalIndexes);
        }
    }
    totalIndexes = indices.size();
}
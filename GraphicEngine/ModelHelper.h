#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>     

#include "FrameResource.h"

//void ProcessMeshIndices(aiMesh* mesh, const aiScene* scene) {
//    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; j++) {
//            indices.push_back(face.mIndices[j]);
//        }
//    }
//}
//
//void ProcessMeshVertices(aiMesh* mesh, const aiScene* scene, UINT& totalVertexCount, std::vector<Vertex> vertices) {
//    for (unsigned int j = 0; j < mesh->mNumVertices; j++)
//    {
//        aiVector3D vertex = mesh->mVertices[j];
//        aiVector3D normal = mesh->mNormals[j];
//        aiVector3D texCoord = mesh->mTextureCoords[0][j];
//
//        vertices[j + totalVertexCount].Pos = DirectX::XMFLOAT3{ vertex.x, vertex.y, vertex.z };
//        vertices[j + totalVertexCount].Normal = DirectX::XMFLOAT3{ normal.x, normal.y, normal.z };
//        vertices[j + totalVertexCount].TexC = DirectX::XMFLOAT2{ texCoord.x, texCoord.y };
//    }
//    totalVertexCount += mesh->mNumVertices;
//}
//
//void ProcessNode(aiNode* node, const aiScene* scene) {
//    // Обработка всех мешей в текущей ноде
//    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
//        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//        ProcessMeshIndices(mesh, scene);
//        ProcessMeshVertices(mesh, scene, total)
//    }
//
//    // Рекурсивный обход дочерних нод
//    for (unsigned int i = 0; i < node->mNumChildren; i++) {
//        ProcessNode(node->mChildren[i], scene);
//    }
//}
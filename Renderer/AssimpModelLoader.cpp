#include "AssimpModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace BSE
{
    static glm::vec3 AiToGlm(const aiVector3D& v) { return glm::vec3(v.x, v.y, v.z); }
    static glm::vec2 AiToGlm2(const aiVector3D& v) { return glm::vec2(v.x, v.y); }

    bool LoadModelWithAssimp(const std::string& filepath, std::vector<MeshData>& outMeshes)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality);

        if (!scene || !scene->HasMeshes())
        {
            std::cerr << "Assimp failed to load or no meshes: " << filepath << " -> "
                      << (importer.GetErrorString() ? importer.GetErrorString() : "(no error)") << std::endl;
            return false;
        }

        outMeshes.clear();
        outMeshes.reserve(scene->mNumMeshes);

        for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi)
        {
            aiMesh* am = scene->mMeshes[mi];
            MeshData md;
            md.name = am->mName.C_Str();
            md.transform = glm::mat4(1.0f);

            md.positions.resize(am->mNumVertices);
            if (am->HasNormals()) md.normals.resize(am->mNumVertices);
            if (am->HasTextureCoords(0)) md.uvs.resize(am->mNumVertices);

            for (unsigned int v = 0; v < am->mNumVertices; ++v)
            {
                md.positions[v] = AiToGlm(am->mVertices[v]);
                if (am->HasNormals()) md.normals[v] = AiToGlm(am->mNormals[v]);
                if (am->HasTextureCoords(0)) md.uvs[v] = AiToGlm2(am->mTextureCoords[0][v]);
            }

            // indices
            if (am->mNumFaces > 0)
            {
                for (unsigned int f = 0; f < am->mNumFaces; ++f)
                {
                    aiFace &face = am->mFaces[f];
                    for (unsigned int k = 0; k < face.mNumIndices; ++k)
                        md.indices.push_back(face.mIndices[k]);
                }
            }

            outMeshes.push_back(std::move(md));
        }

        return true;
    }
}

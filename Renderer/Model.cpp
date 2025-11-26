#include "Model.h"
#include "AssimpModelLoader.h"
#include "Shader.h"
#include <sstream>

namespace BSE
{
    bool ModelLoader::Load(const std::string& filepath)
    {
        Unload();

        std::vector<MeshData> meshes;
        if (BSE::LoadModelWithAssimp(filepath, meshes))
        {
            m_meshes = std::move(meshes);
            return true;
        }

        std::cerr << "Assimp failed to load model: " << filepath << std::endl;
        return false;
    }

    void ModelLoader::Unload()
    {
        m_meshes.clear();
    }

    bool ModelLoader::LoadFromMeshes(const std::vector<MeshData>& meshes)
    {
        Unload();
        m_meshes = meshes;
        return true;
    }

    void ModelProcessor::Process(const std::vector<MeshData>& meshes)
    {
        Release();

        for (const MeshData& mesh : meshes)
        {
            RenderMesh rmesh;
            rmesh.indexCount = static_cast<uint32_t>(mesh.indices.size());

            glGenVertexArrays(1, &rmesh.VAO);
            glGenBuffers(1, &rmesh.VBO);
            glGenBuffers(1, &rmesh.EBO);

            glBindVertexArray(rmesh.VAO);

            std::vector<float> vertexData;
            vertexData.reserve(mesh.positions.size() * 8);
            for (size_t i = 0; i < mesh.positions.size(); ++i)
            {
                const glm::vec3& p = mesh.positions[i];
                const glm::vec3& n = (i < mesh.normals.size()) ? mesh.normals[i] : glm::vec3(0.0f, 1.0f, 0.0f);
                const glm::vec2& uv = (i < mesh.uvs.size()) ? mesh.uvs[i] : glm::vec2(0.0f, 0.0f);

                vertexData.push_back(p.x);
                vertexData.push_back(p.y);
                vertexData.push_back(p.z);

                vertexData.push_back(n.x);
                vertexData.push_back(n.y);
                vertexData.push_back(n.z);

                vertexData.push_back(uv.x);
                vertexData.push_back(uv.y);
            }

            glBindBuffer(GL_ARRAY_BUFFER, rmesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rmesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            rmesh.transform = mesh.transform;

            m_renderMeshes.push_back(rmesh);
        }
    }

    void ModelProcessor::Release()
    {
        for (RenderMesh& mesh : m_renderMeshes)
        {
            if (mesh.VAO) glDeleteVertexArrays(1, &mesh.VAO);
            if (mesh.VBO) glDeleteBuffers(1, &mesh.VBO);
            if (mesh.EBO) glDeleteBuffers(1, &mesh.EBO);
        }
        m_renderMeshes.clear();
    }

    void ModelRenderer::Render(const std::vector<RenderMesh>& meshes, const glm::mat4& viewProjMatrix, GLuint shaderProgram)
    {
        if (shaderProgram == 0) return;

        GLint locMVP = glGetUniformLocation(shaderProgram, "uMVP");
        GLint locModel = glGetUniformLocation(shaderProgram, "uModel");
        GLint locNormalMat = glGetUniformLocation(shaderProgram, "uNormalMatrix");

        for (const RenderMesh& mesh : meshes)
        {
            glm::mat4 mvp = viewProjMatrix * mesh.transform;
            if (locMVP >= 0)
                glUniformMatrix4fv(locMVP, 1, GL_FALSE, &mvp[0][0]);

            if (locModel >= 0)
                glUniformMatrix4fv(locModel, 1, GL_FALSE, &mesh.transform[0][0]);

            if (locNormalMat >= 0)
            {
                glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mesh.transform)));
                glUniformMatrix3fv(locNormalMat, 1, GL_FALSE, &normalMat[0][0]);
            }

            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    bool Model::LoadFromFile(const std::string& filepath)
    {
        Unload();

        if (!m_loader.Load(filepath))
        {
            return false;
        }

        const auto& meshes = m_loader.GetMeshes();
        m_processor.Process(meshes);

        UpdateRenderTransforms();

        return true;
    }

    bool Model::LoadFromMeshes(const std::vector<MeshData>& meshes)
    {
        Unload();

        if (!m_loader.LoadFromMeshes(meshes))
        {
            return false;
        }

        const auto& m = m_loader.GetMeshes();
        m_processor.Process(m);

        UpdateRenderTransforms();

        return true;
    }

    void Model::Unload()
    {
        m_processor.Release();
        m_loader.Unload();
    }

    void Model::SetMeshPosition(size_t meshIndex, const glm::vec3& pos)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Position = pos;
        UpdateRenderTransforms();
    }

    void Model::TranslateMesh(size_t meshIndex, const glm::vec3& delta)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Position += delta;
        UpdateRenderTransforms();
    }

    void Model::SetMeshRotation(size_t meshIndex, const glm::quat& rot)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Rotation = rot;
        UpdateRenderTransforms();
    }

    void Model::RotateMesh(size_t meshIndex, const glm::quat& delta)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Rotation = glm::normalize(delta * meshes[meshIndex].Rotation);
        UpdateRenderTransforms();
    }

    void Model::SetMeshScale(size_t meshIndex, const glm::vec3& scale)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Scale = scale;
        UpdateRenderTransforms();
    }

    void Model::RescaleMesh(size_t meshIndex, const glm::vec3& factor)
    {
        auto& meshes = m_loader.GetMeshesMutable();
        if (meshIndex >= meshes.size()) return;
        meshes[meshIndex].Scale *= factor;
        UpdateRenderTransforms();
    }

    void Model::UpdateRenderTransforms()
    {
        glm::mat4 modelTRS = GetModelTRSMatrix();

        const auto& meshes = m_loader.GetMeshes();
        auto& rmeshes = m_processor.GetRenderMeshesMutable();

        size_t count = std::min(meshes.size(), rmeshes.size());
        for (size_t i = 0; i < count; ++i)
        {
            rmeshes[i].transform = meshes[i].GetFinalTransform(modelTRS);
        }
    }

    void Model::Render(ModelRenderer& renderer, const glm::mat4& viewProjMatrix, GLuint shaderProgram)
    {
        renderer.Render(m_processor.GetRenderMeshes(), viewProjMatrix, shaderProgram);
    }
}

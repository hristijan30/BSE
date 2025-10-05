#include "Model.h"
#include "Shader.h"

namespace BSE
{
    bool ModelLoader::Load(const std::string& filepath)
    {
        Unload();

        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open mesh file: " << filepath << std::endl;
            return false;
        }

        char header[6] = {};
        file.read(header, 6);
        if (std::strncmp(header, "BMESH", 5) != 0)
        {
            std::cerr << "Invalid mesh file header: " << filepath << std::endl;
            return false;
        }

        uint8_t version = 0;
        file.read(reinterpret_cast<char*>(&version), sizeof(uint8_t));
        if (version != 1)
        {
            std::cerr << "Unsupported mesh version: " << int(version) << std::endl;
            return false;
        }

        uint32_t meshCount = 0;
        file.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));
        m_meshes.resize(meshCount);

        auto read_string = [&file]() -> std::string {
            uint16_t len = 0;
            file.read(reinterpret_cast<char*>(&len), sizeof(uint16_t));
            std::string s(len, '\0');
            if (len > 0)
                file.read(&s[0], len);
            return s;
        };

        for (uint32_t i = 0; i < meshCount; ++i)
        {
            MeshData& mesh = m_meshes[i];
            mesh.name = read_string();

            float mat[16];
            file.read(reinterpret_cast<char*>(mat), sizeof(float) * 16);
            mesh.transform = glm::make_mat4(mat);

            uint32_t vertexCount = 0, indexCount = 0;
            file.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
            file.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

            mesh.positions.resize(vertexCount);
            mesh.normals.resize(vertexCount);
            mesh.uvs.resize(vertexCount);
            mesh.indices.resize(indexCount);

            for (uint32_t v = 0; v < vertexCount; ++v)
            {
                file.read(reinterpret_cast<char*>(&mesh.positions[v]), sizeof(glm::vec3));
                file.read(reinterpret_cast<char*>(&mesh.normals[v]), sizeof(glm::vec3));
                file.read(reinterpret_cast<char*>(&mesh.uvs[v]), sizeof(glm::vec2));
            }

            if (indexCount > 0)
                file.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint32_t) * indexCount);
        }

        file.close();
        return true;
    }

    void ModelLoader::Unload()
    {
        m_meshes.clear();
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

        GLint loc = glGetUniformLocation(shaderProgram, "uMVP");

        for (const RenderMesh& mesh : meshes)
        {
            glm::mat4 mvp = viewProjMatrix * mesh.transform;
            if (loc >= 0)
                glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp[0][0]);

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

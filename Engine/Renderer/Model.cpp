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
            file.read(s.data(), len);
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
            rmesh.transform = mesh.transform;

            glGenVertexArrays(1, &rmesh.VAO);
            glGenBuffers(1, &rmesh.VBO);
            glGenBuffers(1, &rmesh.EBO);

            glBindVertexArray(rmesh.VAO);

            std::vector<float> vertexData;
            for (size_t i = 0; i < mesh.positions.size(); ++i)
            {
                vertexData.push_back(mesh.positions[i].x);
                vertexData.push_back(mesh.positions[i].y);
                vertexData.push_back(mesh.positions[i].z);

                vertexData.push_back(mesh.normals[i].x);
                vertexData.push_back(mesh.normals[i].y);
                vertexData.push_back(mesh.normals[i].z);

                vertexData.push_back(mesh.uvs[i].x);
                vertexData.push_back(mesh.uvs[i].y);
            }

            glBindBuffer(GL_ARRAY_BUFFER, rmesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rmesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
        glUseProgram(shaderProgram);

        GLint loc = glGetUniformLocation(shaderProgram, "uMVP");

        for (const RenderMesh& mesh : meshes)
        {
            glm::mat4 mvp = viewProjMatrix * mesh.transform;
            glUniformMatrix4fv(loc, 1, GL_FALSE, &mvp[0][0]);

            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }

        glUseProgram(0);
    }
}

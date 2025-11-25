#include "Model.h"
#include "Shader.h"
#include <sstream>
#include "../STB/stb_image.h"

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

        uint8_t nextByte = 0;
        file.read(reinterpret_cast<char*>(&nextByte), sizeof(uint8_t));

        auto read_from_stream = [&m = m_meshes](std::istream& in) -> bool {
            uint32_t meshCount = 0;
            in.read(reinterpret_cast<char*>(&meshCount), sizeof(uint32_t));

            const uint32_t MAX_MESHES = 10000;
            if (meshCount > MAX_MESHES)
            {
                std::cerr << "Mesh file contains too many meshes: " << meshCount << " (limit " << MAX_MESHES << ")" << std::endl;
                return false;
            }

            try {
                m.resize(meshCount);
            } catch (const std::bad_alloc&) {
                std::cerr << "Failed to allocate mesh list (too large): " << meshCount << std::endl;
                return false;
            }

            auto read_string = [&in]() -> std::string {
                uint16_t len = 0;
                in.read(reinterpret_cast<char*>(&len), sizeof(uint16_t));
                std::string s(len, '\0');
                if (len > 0)
                    in.read(&s[0], len);
                return s;
            };

            for (uint32_t i = 0; i < meshCount; ++i)
            {
                MeshData& mesh = m[i];
                mesh.name = read_string();

                float mat[16];
                in.read(reinterpret_cast<char*>(mat), sizeof(float) * 16);
                mesh.transform = glm::make_mat4(mat);

                uint32_t vertexCount = 0, indexCount = 0;
                in.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
                in.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

                const uint32_t MAX_VERTS = 10000000;
                const uint32_t MAX_INDICES = 30000000;

                if (vertexCount > MAX_VERTS || indexCount > MAX_INDICES)
                {
                    std::cerr << "Mesh has unreasonable vertex/index counts: v=" << vertexCount << " i=" << indexCount << std::endl;
                    return false;
                }

                try {
                    mesh.positions.resize(vertexCount);
                    mesh.normals.resize(vertexCount);
                    mesh.uvs.resize(vertexCount);
                    mesh.indices.resize(indexCount);
                } catch (const std::bad_alloc&) {
                    std::cerr << "Failed to allocate mesh vertex/index buffers: v=" << vertexCount << " i=" << indexCount << std::endl;
                    return false;
                }

                for (uint32_t v = 0; v < vertexCount; ++v)
                {
                    in.read(reinterpret_cast<char*>(&mesh.positions[v]), sizeof(glm::vec3));
                    in.read(reinterpret_cast<char*>(&mesh.normals[v]), sizeof(glm::vec3));
                    in.read(reinterpret_cast<char*>(&mesh.uvs[v]), sizeof(glm::vec2));
                }

                if (indexCount > 0)
                    in.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint32_t) * indexCount);
            }

            return true;
        };

        const uint8_t FLAG_COMPRESSED = 1;

        if (nextByte == 1)
        {
            if (!read_from_stream(file))
                return false;
        }
        else if (nextByte == 2)
        {
            uint8_t flags = 0;
            file.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));

            if (flags & FLAG_COMPRESSED)
            {
                    std::streampos posAfterFlags = file.tellg();

                    uint32_t compressedSize = 0, originalSize = 0;
                    file.read(reinterpret_cast<char*>(&compressedSize), sizeof(uint32_t));
                    file.read(reinterpret_cast<char*>(&originalSize), sizeof(uint32_t));

                    const uint32_t MAX_COMPRESSED = 100u * 1024u * 1024u;
                    const uint32_t MAX_ORIGINAL = 500u * 1024u * 1024u;

                    if (compressedSize == 0 || originalSize == 0 || compressedSize > MAX_COMPRESSED || originalSize > MAX_ORIGINAL)
                    {
                        std::cerr << "Compressed mesh payload sizes invalid or too large: comp=" << compressedSize << " orig=" << originalSize << ". Falling back to uncompressed parsing." << std::endl;
                        file.clear();
                        file.seekg(posAfterFlags);
                        if (!read_from_stream(file)) return false;
                        return true;
                    }

                    std::vector<unsigned char> compBuf;
                    std::vector<unsigned char> decompBuf;
                    try {
                        compBuf.resize(compressedSize);
                        file.read(reinterpret_cast<char*>(compBuf.data()), compressedSize);
                        decompBuf.resize(originalSize);
                    } catch (const std::bad_alloc&) {
                        std::cerr << "Failed to allocate compressed/decompressed buffers (too large). Falling back to uncompressed parsing." << std::endl;
                        file.clear();
                        file.seekg(posAfterFlags);
                        if (!read_from_stream(file)) return false;
                        return true;
                    }

                    int dec = stbi_zlib_decode_buffer(reinterpret_cast<char*>(decompBuf.data()), (int)originalSize,
                                                      reinterpret_cast<char*>(compBuf.data()), (int)compressedSize);
                    if (dec <= 0)
                    {
                        std::cerr << "Failed to decompress mesh payload: " << filepath << ". Falling back to uncompressed parsing." << std::endl;
                        file.clear();
                        file.seekg(posAfterFlags);
                        if (!read_from_stream(file)) return false;
                        return true;
                    }

                    std::string payload(reinterpret_cast<char*>(decompBuf.data()), originalSize);
                    std::istringstream mem(payload);
                    if (!read_from_stream(mem))
                        return false;
            }
            else
            {
                if (!read_from_stream(file))
                    return false;
            }
        }
        else
        {
            uint8_t flags = nextByte;

            if (flags & FLAG_COMPRESSED)
            {
                std::streampos posAfterFlags = file.tellg();

                uint32_t compressedSize = 0, originalSize = 0;
                file.read(reinterpret_cast<char*>(&compressedSize), sizeof(uint32_t));
                file.read(reinterpret_cast<char*>(&originalSize), sizeof(uint32_t));

                const uint32_t MAX_COMPRESSED = 100u * 1024u * 1024u;
                const uint32_t MAX_ORIGINAL = 500u * 1024u * 1024u;

                if (compressedSize == 0 || originalSize == 0 || compressedSize > MAX_COMPRESSED || originalSize > MAX_ORIGINAL)
                {
                    std::cerr << "Compressed mesh payload sizes invalid or too large: comp=" << compressedSize << " orig=" << originalSize << ". Falling back to uncompressed parsing." << std::endl;
                    file.clear();
                    file.seekg(posAfterFlags);
                    if (!read_from_stream(file)) return false;
                    return true;
                }

                std::vector<unsigned char> compBuf;
                std::vector<unsigned char> decompBuf;
                try {
                    compBuf.resize(compressedSize);
                    file.read(reinterpret_cast<char*>(compBuf.data()), compressedSize);
                    decompBuf.resize(originalSize);
                } catch (const std::bad_alloc&) {
                    std::cerr << "Failed to allocate compressed/decompressed buffers (too large). Falling back to uncompressed parsing." << std::endl;
                    file.clear();
                    file.seekg(posAfterFlags);
                    if (!read_from_stream(file)) return false;
                    return true;
                }

                int dec = stbi_zlib_decode_buffer(reinterpret_cast<char*>(decompBuf.data()), (int)originalSize,
                                                  reinterpret_cast<char*>(compBuf.data()), (int)compressedSize);
                if (dec <= 0)
                {
                    std::cerr << "Failed to decompress mesh payload: " << filepath << ". Falling back to uncompressed parsing." << std::endl;
                    file.clear();
                    file.seekg(posAfterFlags);
                    if (!read_from_stream(file)) return false;
                    return true;
                }

                std::string payload(reinterpret_cast<char*>(decompBuf.data()), originalSize);
                std::istringstream mem(payload);
                if (!read_from_stream(mem))
                    return false;
            }
            else
            {
                if (!read_from_stream(file))
                    return false;
            }
        }

        file.close();
        return true;
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

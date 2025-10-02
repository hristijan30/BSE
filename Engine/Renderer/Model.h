#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    struct DLL_EXPORT MeshData
    {
        std::string name;
        glm::mat4 transform;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;
    };

    struct DLL_EXPORT RenderMesh
    {
        GLuint VAO = 0;
        GLuint VBO = 0;
        GLuint EBO = 0;
        uint32_t indexCount = 0;
        glm::mat4 transform = glm::mat4(1.0f);
    };

    class DLL_EXPORT ModelLoader
    {
    public:
        ModelLoader() = default;
        ~ModelLoader() { Unload(); }

        bool Load(const std::string& filepath);
        void Unload();

        const std::vector<MeshData>& GetMeshes() const { return m_meshes; }

    private:
        std::vector<MeshData> m_meshes;
    };

    class DLL_EXPORT ModelProcessor
    {
    public:
        ModelProcessor() = default;
        ~ModelProcessor() { Release(); }

        void Process(const std::vector<MeshData>& meshes);
        void Release();

        const std::vector<RenderMesh>& GetRenderMeshes() const { return m_renderMeshes; }

    private:
        std::vector<RenderMesh> m_renderMeshes;
    };

    class DLL_EXPORT ModelRenderer
    {
    public:
        ModelRenderer() = default;

        void Render(const std::vector<RenderMesh>& meshes, const glm::mat4& viewProjMatrix, GLuint shaderProgram);
    };
}

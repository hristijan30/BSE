#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    struct DLL_EXPORT MeshData
    {
        std::string name;

        glm::mat4 transform = glm::mat4(1.0f);

        glm::vec3 Position = glm::vec3(0.0f);
        glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;

        glm::mat4 GetLocalTRS() const
        {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
            glm::mat4 R = glm::mat4_cast(Rotation);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);
            return T * R * S;
        }

        glm::mat4 GetFinalTransform(const glm::mat4& modelTRS) const
        {
            return modelTRS * transform * GetLocalTRS();
        }
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
        std::vector<MeshData>& GetMeshesMutable() { return m_meshes; }

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
        std::vector<RenderMesh>& GetRenderMeshesMutable() { return m_renderMeshes; }

    private:
        std::vector<RenderMesh> m_renderMeshes;
    };

    class DLL_EXPORT ModelRenderer
    {
    public:
        ModelRenderer() = default;

        void Render(const std::vector<RenderMesh>& meshes, const glm::mat4& viewProjMatrix, GLuint shaderProgram);
    };

    class DLL_EXPORT Model
    {
    public:
        Model() = default;
        ~Model() { Unload(); }

        bool LoadFromFile(const std::string& filepath);
        void Unload();

        void SetPosition(const glm::vec3& pos) { m_position = pos; UpdateRenderTransforms(); }
        void SetRotation(const glm::quat& rot) { m_rotation = rot; UpdateRenderTransforms(); }
        void SetScale(const glm::vec3& scale) { m_scale = scale; UpdateRenderTransforms(); }

        void Translate(const glm::vec3& delta) { m_position += delta; UpdateRenderTransforms(); }
        void Rotate(const glm::quat& delta) { m_rotation = glm::normalize(delta * m_rotation); UpdateRenderTransforms(); }
        void Rescale(const glm::vec3& factor) { m_scale *= factor; UpdateRenderTransforms(); }

        glm::vec3 GetPosition() const { return m_position; }
        glm::quat GetRotation() const { return m_rotation; }
        glm::vec3 GetScale() const { return m_scale; }

        void SetMeshPosition(size_t meshIndex, const glm::vec3& pos);
        void TranslateMesh(size_t meshIndex, const glm::vec3& delta);
        void SetMeshRotation(size_t meshIndex, const glm::quat& rot);
        void RotateMesh(size_t meshIndex, const glm::quat& delta);
        void SetMeshScale(size_t meshIndex, const glm::vec3& scale);
        void RescaleMesh(size_t meshIndex, const glm::vec3& factor);

        void UpdateRenderTransforms();

        void Render(ModelRenderer& renderer, const glm::mat4& viewProjMatrix, GLuint shaderProgram);

        const std::vector<MeshData>& GetMeshes() const { return m_loader.GetMeshes(); }
        const std::vector<RenderMesh>& GetRenderMeshes() const { return m_processor.GetRenderMeshes(); }

    private:
        glm::mat4 GetModelTRSMatrix() const
        {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), m_position);
            glm::mat4 R = glm::mat4_cast(m_rotation);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), m_scale);
            return T * R * S;
        }

        ModelLoader m_loader;
        ModelProcessor m_processor;

        glm::vec3 m_position = glm::vec3(0.0f);
        glm::quat m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_scale = glm::vec3(1.0f);
    };
}

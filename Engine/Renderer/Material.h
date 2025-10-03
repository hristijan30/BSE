#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    class DLL_EXPORT Texture2D
    {
    public:
        Texture2D() = default;
        Texture2D(const std::string& path, bool srgb = true);
        ~Texture2D();

        bool LoadFromFile(const std::string& path, bool srgb = true);
        void Bind(GLuint slot = 0) const;
        void Unbind() const;

        GLuint GetID() const { return m_id; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        bool IsLoaded() const { return m_loaded; }

    private:
        GLuint m_id = 0;
        int m_width = 0;
        int m_height = 0;
        bool m_loaded = false;
    };

    class DLL_EXPORT Material
    {
    public:
        Material() = default;
        ~Material() = default;

        bool LoadFromFile(const std::string& filepath); // Extention: .material
        void Bind(GLuint shaderProgram) const;

        const Texture2D* GetDiffuseMap() const { return m_diffuse.get(); }
        const Texture2D* GetNormalMap() const { return m_normal.get(); }
        const Texture2D* GetRoughnessMap() const { return m_roughness.get(); }
        const Texture2D* GetMetallicMap() const { return m_metallic.get(); }
        const Texture2D* GetAOMap() const { return m_ao.get(); }
        const Texture2D* GetEmissiveMap() const { return m_emissive.get(); }

        glm::vec3 BaseColor = glm::vec3(1.0f);
        glm::vec3 EmissionColor = glm::vec3(0.0f);
        float Metallic = 0.0f;
        float Roughness = 0.5f;
        float Transparency = 0.0f;
        float EmissionStrength = 0.0f;
        float SpecularStrength = 0.5f;

    private:
        std::unique_ptr<Texture2D> m_diffuse;
        std::unique_ptr<Texture2D> m_normal;
        std::unique_ptr<Texture2D> m_roughness;
        std::unique_ptr<Texture2D> m_metallic;
        std::unique_ptr<Texture2D> m_ao;
        std::unique_ptr<Texture2D> m_emissive;

        bool parseMaterialFile(const std::string& filepath);
    };
}

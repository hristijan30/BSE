#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "OpenGL.h"
#include "Texture2D.h"

namespace BSE
{
    class DLL_EXPORT Material
    {
    public:
        Material() = default;
        ~Material() = default;

        bool LoadFromFile(const std::string& filepath);
        bool ParseMaterialFile(const std::string& filepath);
        void FinalizeTexturesFromImageData(const std::unordered_map<std::string, ImageData>& images);
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

        std::string diffusePath;
        std::string normalPath;
        std::string roughnessPath;
        std::string metallicPath;
        std::string aoPath;
        std::string emissivePath;

    private:
        std::unique_ptr<Texture2D> m_diffuse;
        std::unique_ptr<Texture2D> m_normal;
        std::unique_ptr<Texture2D> m_roughness;
        std::unique_ptr<Texture2D> m_metallic;
        std::unique_ptr<Texture2D> m_ao;
        std::unique_ptr<Texture2D> m_emissive;

        bool parseMaterialFileInternal(const std::string& filepath);
    };
}

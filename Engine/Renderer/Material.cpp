#include "Material.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace BSE
{
    bool Material::LoadFromFile(const std::string& filepath)
    {
        if (!parseMaterialFileInternal(filepath))
            return false;

        if (!diffusePath.empty()) m_diffuse = std::make_unique<Texture2D>(diffusePath, true);
        if (!normalPath.empty()) m_normal = std::make_unique<Texture2D>(normalPath, false);
        if (!roughnessPath.empty()) m_roughness = std::make_unique<Texture2D>(roughnessPath, false);
        if (!metallicPath.empty()) m_metallic = std::make_unique<Texture2D>(metallicPath, false);
        if (!aoPath.empty()) m_ao = std::make_unique<Texture2D>(aoPath, false);
        if (!emissivePath.empty()) m_emissive = std::make_unique<Texture2D>(emissivePath, false);

        return true;
    }

    bool Material::ParseMaterialFile(const std::string& filepath)
    {
        return parseMaterialFileInternal(filepath);
    }

    bool Material::parseMaterialFileInternal(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "[Material] Failed to open file: " << filepath << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string key;
            if (!(iss >> key)) continue;

            if (key == "diffuse") { iss >> diffusePath; if (diffusePath == "null") diffusePath.clear(); }
            else if (key == "normal") { iss >> normalPath; if (normalPath == "null") normalPath.clear(); }
            else if (key == "roughness") { iss >> roughnessPath; if (roughnessPath == "null") roughnessPath.clear(); }
            else if (key == "metallic") { iss >> metallicPath; if (metallicPath == "null") metallicPath.clear(); }
            else if (key == "ao") { iss >> aoPath; if (aoPath == "null") aoPath.clear(); }
            else if (key == "emissive") { iss >> emissivePath; if (emissivePath == "null") emissivePath.clear(); }
            else if (key == "BaseColor") { iss >> BaseColor.r >> BaseColor.g >> BaseColor.b; }
            else if (key == "EmissionColor") { iss >> EmissionColor.r >> EmissionColor.g >> EmissionColor.b; }
            else if (key == "Metallic") { iss >> Metallic; }
            else if (key == "Roughness") { iss >> Roughness; }
            else if (key == "Transparency") { iss >> Transparency; }
            else if (key == "EmissionStrength") { iss >> EmissionStrength; }
            else if (key == "SpecularStrength") { iss >> SpecularStrength; }
        }

        return true;
    }

    void Material::FinalizeTexturesFromImageData(const std::unordered_map<std::string, ImageData>& images)
    {
        auto findAndCreate = [&](const std::string& path, std::unique_ptr<Texture2D>& slot, bool srgb)
        {
            if (path.empty()) return;
            auto it = images.find(path);
            if (it != images.end())
            {
                slot = std::make_unique<Texture2D>();
                if (!slot->CreateFromImageData(it->second, srgb))
                {
                    std::cerr << "[Material] Failed to CreateFromImageData for: " << path << std::endl;
                    slot.reset();
                }
            }
            else
            {
                slot = std::make_unique<Texture2D>();
                if (!slot->LoadFromFile(path, srgb))
                {
                    std::cerr << "[Material] Fallback LoadFromFile failed for: " << path << std::endl;
                    slot.reset();
                }
            }
        };

        findAndCreate(diffusePath, m_diffuse, true);
        findAndCreate(normalPath, m_normal, false);
        findAndCreate(roughnessPath, m_roughness, false);
        findAndCreate(metallicPath, m_metallic, false);
        findAndCreate(aoPath, m_ao, false);
        findAndCreate(emissivePath, m_emissive, false);
    }

    void Material::Bind(GLuint shaderProgram) const
    {
        if (shaderProgram == 0) return;

        glUseProgram(shaderProgram);

        GLint loc;
        loc = glGetUniformLocation(shaderProgram, "u_BaseColor");
        if (loc >= 0) glUniform3fv(loc, 1, &BaseColor[0]);

        loc = glGetUniformLocation(shaderProgram, "u_EmissionColor");
        if (loc >= 0) glUniform3fv(loc, 1, &EmissionColor[0]);

        loc = glGetUniformLocation(shaderProgram, "u_Metallic");
        if (loc >= 0) glUniform1f(loc, Metallic);

        loc = glGetUniformLocation(shaderProgram, "u_Roughness");
        if (loc >= 0) glUniform1f(loc, Roughness);

        loc = glGetUniformLocation(shaderProgram, "u_Transparency");
        if (loc >= 0) glUniform1f(loc, Transparency);

        loc = glGetUniformLocation(shaderProgram, "u_EmissionStrength");
        if (loc >= 0) glUniform1f(loc, EmissionStrength);

        loc = glGetUniformLocation(shaderProgram, "u_SpecularStrength");
        if (loc >= 0) glUniform1f(loc, SpecularStrength);

        int slot = 0;
        if (m_diffuse) { m_diffuse->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_DiffuseMap"), slot); slot++; }
        if (m_normal) { m_normal->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_NormalMap"), slot); slot++; }
        if (m_roughness) { m_roughness->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_RoughnessMap"), slot); slot++; }
        if (m_metallic) { m_metallic->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_MetallicMap"), slot); slot++; }
        if (m_ao) { m_ao->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_AOMap"), slot); slot++; }
        if (m_emissive) { m_emissive->Bind(slot); glUniform1i(glGetUniformLocation(shaderProgram, "u_EmissiveMap"), slot); slot++; }
    }
}

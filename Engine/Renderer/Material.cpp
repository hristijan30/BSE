#include "Material.h"
#include "../STB/stb_image.h"

namespace BSE
{
    Texture2D::Texture2D(const std::string& path, bool srgb)
    {
        LoadFromFile(path, srgb);
    }

    Texture2D::~Texture2D()
    {
        if (m_id != 0)
            glDeleteTextures(1, &m_id);
    }

    bool Texture2D::LoadFromFile(const std::string& path, bool srgb)
    {
        if (m_id != 0) {
            glDeleteTextures(1, &m_id);
            m_id = 0;
        }

        int channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
        if (!data)
        {
            std::cerr << "[Texture2D] Failed to load: " << path << std::endl;
            return false;
        }

        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 3) format = srgb ? GL_SRGB : GL_RGB;
        else if (channels == 4) format = srgb ? GL_SRGB_ALPHA : GL_RGBA;

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0,
                     (channels == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);
        m_loaded = true;
        return true;
    }

    void Texture2D::Bind(GLuint slot) const
    {
        if (m_loaded)
        {
            glActiveTexture(GL_TEXTURE0 + slot);
            glBindTexture(GL_TEXTURE_2D, m_id);
        }
    }

    void Texture2D::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool Material::LoadFromFile(const std::string& filepath)
    {
        return parseMaterialFile(filepath);
    }

    bool Material::parseMaterialFile(const std::string& filepath)
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
            std::string key, value;
            if (!(iss >> key >> value)) continue;

            if (key == "diffuse" && value != "null")
                m_diffuse = std::make_unique<Texture2D>(value, true);
            else if (key == "normal" && value != "null")
                m_normal = std::make_unique<Texture2D>(value, false);
            else if (key == "roughness" && value != "null")
                m_roughness = std::make_unique<Texture2D>(value, false);
            else if (key == "metallic" && value != "null")
                m_metallic = std::make_unique<Texture2D>(value, false);
            else if (key == "ao" && value != "null")
                m_ao = std::make_unique<Texture2D>(value, false);
            else if (key == "emissive" && value != "null")
                m_emissive = std::make_unique<Texture2D>(value, false);
            else if (key == "BaseColor")
                iss >> BaseColor.r >> BaseColor.g >> BaseColor.b;
            else if (key == "EmissionColor")
                iss >> EmissionColor.r >> EmissionColor.g >> EmissionColor.b;
            else if (key == "Metallic")
                Metallic = std::stof(value);
            else if (key == "Roughness")
                Roughness = std::stof(value);
            else if (key == "Transparency")
                Transparency = std::stof(value);
            else if (key == "EmissionStrength")
                EmissionStrength = std::stof(value);
            else if (key == "SpecularStrength")
                SpecularStrength = std::stof(value);
        }

        return true;
    }

    void Material::Bind(GLuint shaderProgram) const
    {
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

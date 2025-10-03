#include "Texture2D.h"
#include <iostream>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "../STB/stb_image.h"

namespace BSE
{
    bool Texture2D::LoadImageToMemory(const std::string& path, ImageData& out, bool flipVertically)
    {
        stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);
        unsigned char* data = stbi_load(path.c_str(), &out.width, &out.height, &out.channels, 0);
        if (!data)
        {
            std::cerr << "[Texture2D] Failed to load image to memory: " << path << std::endl;
            return false;
        }
        size_t size = static_cast<size_t>(out.width) * static_cast<size_t>(out.height) * static_cast<size_t>(out.channels);
        out.pixels.resize(size);
        std::memcpy(out.pixels.data(), data, size);
        stbi_image_free(data);
        return true;
    }

    bool Texture2D::CreateFromImageData(const ImageData& data, bool srgb)
    {
        if (m_id != 0)
        {
            glDeleteTextures(1, &m_id);
            m_id = 0;
            m_loaded = false;
        }

        if (data.pixels.empty() || data.width <= 0 || data.height <= 0) return false;

        GLenum internalFormat = GL_RGB;
        GLenum format = GL_RGB;

        if (data.channels == 1) { internalFormat = format = GL_RED; }
        else if (data.channels == 3) { internalFormat = srgb ? GL_SRGB : GL_RGB; format = GL_RGB; }
        else if (data.channels == 4) { internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA; format = GL_RGBA; }
        else { return false; }

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, data.width, data.height, 0, format, GL_UNSIGNED_BYTE, data.pixels.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        m_width = data.width;
        m_height = data.height;
        m_channels = data.channels;
        m_loaded = true;
        return true;
    }

    bool Texture2D::LoadFromFile(const std::string& path, bool srgb)
    {
        ImageData data;
        if (!LoadImageToMemory(path, data, true)) return false;
        return CreateFromImageData(data, srgb);
    }

    Texture2D::Texture2D(const std::string& path, bool srgb)
    {
        LoadFromFile(path, srgb);
    }

    Texture2D::~Texture2D()
    {
        if (m_id != 0)
            glDeleteTextures(1, &m_id);
    }

    void Texture2D::Bind(GLuint slot) const
    {
        if (!m_loaded) return;
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void Texture2D::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    struct DLL_EXPORT ImageData
    {
        int width = 0;
        int height = 0;
        int channels = 0;
        std::vector<unsigned char> pixels;
    };

    class DLL_EXPORT Texture2D
    {
    public:
        Texture2D() = default;
        explicit Texture2D(const std::string& path, bool srgb = true);
        ~Texture2D();

        static bool LoadImageToMemory(const std::string& path, ImageData& out, bool flipVertically = true);

        bool CreateFromImageData(const ImageData& data, bool srgb = true);

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
        int m_channels = 0;
        bool m_loaded = false;
    };
}

#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    enum class ShaderType {
        Vertex, Fragment, Geometry, Compute, TessControl, TessEval
    };

    inline GLenum ShaderTypeToGLenum(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:      return GL_VERTEX_SHADER;
        case ShaderType::Fragment:    return GL_FRAGMENT_SHADER;
        case ShaderType::Geometry:    return GL_GEOMETRY_SHADER;
        case ShaderType::Compute:     return GL_COMPUTE_SHADER;
        case ShaderType::TessControl: return GL_TESS_CONTROL_SHADER;
        case ShaderType::TessEval:    return GL_TESS_EVALUATION_SHADER;
        default: return GL_FRAGMENT_SHADER;
        }
    }

    class DLL_EXPORT Shader
    {
    public:
        Shader(const std::string& source, ShaderType type);
        ~Shader();

        GLuint GetID() const { return id; }

    private:
        GLuint id = 0;
    };

    class DLL_EXPORT ShaderProgram
    {
    public:
        ShaderProgram(const Shader& vertex, const Shader& fragment,
                      const Shader* geometry = nullptr,
                      const Shader* tessControl = nullptr,
                      const Shader* tessEval = nullptr);
        ~ShaderProgram();

        void Bind() const;
        void Unbind() const;

        void SetUniform(const std::string& name, int value) const;
        void SetUniform(const std::string& name, float value) const;
        void SetUniform(const std::string& name, const glm::mat4& matrix) const;

        GLuint GetID() const { return programID; }

        GLuint Release();

    private:
        GLuint programID = 0;
        bool m_ownsProgram = true;
    };

    class DLL_EXPORT ComputeShaderProgram
    {
    public:
        ComputeShaderProgram(const Shader& compute);
        ~ComputeShaderProgram();

        void Bind() const;
        void Unbind() const;

        void Dispatch(GLuint x, GLuint y, GLuint z) const;

        GLuint GetID() const { return programID; }

    private:
        GLuint programID = 0;
    };
}

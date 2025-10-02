#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include "OpenGL.h"

namespace BSE
{
    enum class ShaderType {
        Vertex, Fragment, Geometry, Compute, TessControl, TessEval
    };

    static GLenum ShaderTypeToGLenum(ShaderType type);

    class DLL_EXPORT Shader
    {
    public:
        Shader(const std::string& source, ShaderType type);
        ~Shader();

        GLuint GetID() const { return id; }

    private:
        GLuint id;
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

    private:
        GLuint programID;
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
        GLuint programID;
    };
}

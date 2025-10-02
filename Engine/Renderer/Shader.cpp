#include "Shader.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace BSE
{
    static GLenum ShaderTypeToGLenum(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:      return GL_VERTEX_SHADER;
        case ShaderType::Fragment:    return GL_FRAGMENT_SHADER;
        case ShaderType::Geometry:    return GL_GEOMETRY_SHADER;
        case ShaderType::Compute:     return GL_COMPUTE_SHADER;
        case ShaderType::TessControl: return GL_TESS_CONTROL_SHADER;
        case ShaderType::TessEval:    return GL_TESS_EVALUATION_SHADER;
        default:                      return 0;
        }
    }

    static std::string ShaderTypeToString(ShaderType type)
    {
        switch (type)
        {
        case ShaderType::Vertex:      return "Vertex";
        case ShaderType::Fragment:    return "Fragment";
        case ShaderType::Geometry:    return "Geometry";
        case ShaderType::Compute:     return "Compute";
        case ShaderType::TessControl: return "TessControl";
        case ShaderType::TessEval:    return "TessEval";
        default:                      return "Unknown";
        }
    }

    static std::string GetShaderInfoLog(GLuint shader)
    {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen <= 0) return std::string();
        std::vector<char> buf(static_cast<size_t>(logLen));
        glGetShaderInfoLog(shader, logLen, nullptr, buf.data());
        return std::string(buf.begin(), buf.end());
    }

    static std::string GetProgramInfoLog(GLuint program)
    {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen <= 0) return std::string();
        std::vector<char> buf(static_cast<size_t>(logLen));
        glGetProgramInfoLog(program, logLen, nullptr, buf.data());
        return std::string(buf.begin(), buf.end());
    }

    Shader::Shader(const std::string& source, ShaderType type)
        : id(0)
    {
        GLenum glType = ShaderTypeToGLenum(type);
        if (glType == 0)
            throw std::runtime_error("Shader: unsupported shader type");

        id = glCreateShader(glType);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        GLint compiled = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
        if (compiled != GL_TRUE)
        {
            std::string log = GetShaderInfoLog(id);
            glDeleteShader(id);
            id = 0;
            std::string err = "Shader compilation failed [" + ShaderTypeToString(type) + "]: " + log;
            std::cerr << err << std::endl;
            throw std::runtime_error(err);
        }
    }

    Shader::~Shader()
    {
        if (id != 0)
        {
            glDeleteShader(id);
            id = 0;
        }
    }

    ShaderProgram::ShaderProgram(const Shader& vertex, const Shader& fragment,
                    const Shader* geometry,
                    const Shader* tessControl,
                    const Shader* tessEval)
        : programID(0)
    {
        programID = glCreateProgram();
        if (programID == 0)
            throw std::runtime_error("Failed to create GL program");

        glAttachShader(programID, vertex.GetID());
        glAttachShader(programID, fragment.GetID());
        if (geometry)    glAttachShader(programID, geometry->GetID());
        if (tessControl) glAttachShader(programID, tessControl->GetID());
        if (tessEval)    glAttachShader(programID, tessEval->GetID());

        glLinkProgram(programID);

        GLint linked = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            std::string log = GetProgramInfoLog(programID);
            std::cerr << "ShaderProgram linking failed (ID " << programID << "): " << log << std::endl;

            glDetachShader(programID, vertex.GetID());
            glDetachShader(programID, fragment.GetID());
            if (geometry)    glDetachShader(programID, geometry->GetID());
            if (tessControl) glDetachShader(programID, tessControl->GetID());
            if (tessEval)    glDetachShader(programID, tessEval->GetID());

            glDeleteProgram(programID);
            programID = 0;

            throw std::runtime_error("ShaderProgram linking failed. See console for details.");
        }

        glDetachShader(programID, vertex.GetID());
        glDetachShader(programID, fragment.GetID());
        if (geometry)    glDetachShader(programID, geometry->GetID());
        if (tessControl) glDetachShader(programID, tessControl->GetID());
        if (tessEval)    glDetachShader(programID, tessEval->GetID());
    }

    ShaderProgram::~ShaderProgram()
    {
        if (programID != 0)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }

    void ShaderProgram::Bind() const { glUseProgram(programID); }
    void ShaderProgram::Unbind() const { glUseProgram(0); }

    void ShaderProgram::SetUniform(const std::string& name, int value) const
    {
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc != -1) glUniform1i(loc, value);
        else std::cerr << "Warning: uniform '" << name << "' not found in ShaderProgram " << programID << std::endl;
    }

    void ShaderProgram::SetUniform(const std::string& name, float value) const
    {
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc != -1) glUniform1f(loc, value);
        else std::cerr << "Warning: uniform '" << name << "' not found in ShaderProgram " << programID << std::endl;
    }

    void ShaderProgram::SetUniform(const std::string& name, const glm::mat4& matrix) const
    {
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
        else std::cerr << "Warning: uniform '" << name << "' not found in ShaderProgram " << programID << std::endl;
    }

    ComputeShaderProgram::ComputeShaderProgram(const Shader& compute)
        : programID(0)
    {
        programID = glCreateProgram();
        if (programID == 0)
            throw std::runtime_error("Failed to create GL compute program");

        glAttachShader(programID, compute.GetID());
        glLinkProgram(programID);

        GLint linked = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            std::string log = GetProgramInfoLog(programID);
            std::cerr << "ComputeShaderProgram linking failed (ID " << programID << "): " << log << std::endl;

            glDetachShader(programID, compute.GetID());
            glDeleteProgram(programID);
            programID = 0;

            throw std::runtime_error("ComputeShaderProgram linking failed. See console for details.");
        }

        glDetachShader(programID, compute.GetID());
    }

    ComputeShaderProgram::~ComputeShaderProgram()
    {
        if (programID != 0)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }

    void ComputeShaderProgram::Bind() const { glUseProgram(programID); }
    void ComputeShaderProgram::Unbind() const { glUseProgram(0); }

    void ComputeShaderProgram::Dispatch(GLuint x, GLuint y, GLuint z) const
    {
        glUseProgram(programID);
        glDispatchCompute(x, y, z);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        glUseProgram(0);
    }
}

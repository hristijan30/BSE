#include "Shader.h"

namespace BSE
{
    static GLuint CompileShaderInternal(const std::string& source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen ? logLen : 1);
            glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
            std::string msg = "Shader compilation failed: ";
            msg += std::string(log.data(), log.size());
            glDeleteShader(shader);
            throw std::runtime_error(msg);
        }
        return shader;
    }

    Shader::Shader(const std::string& source, ShaderType type)
    {
        GLenum glType = ShaderTypeToGLenum(type);
        id = CompileShaderInternal(source, glType);
    }

    Shader::~Shader()
    {
        if (id != 0)
        {
            glDeleteShader(id);
            id = 0;
        }
    }

    static void CheckProgramLinkStatus(GLuint program)
    {
        GLint linked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked)
        {
            GLint logLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen ? logLen : 1);
            glGetProgramInfoLog(program, log.size(), nullptr, log.data());
            std::string msg = "Program link failed: ";
            msg += std::string(log.data(), log.size());
            throw std::runtime_error(msg);
        }
    }

    ShaderProgram::ShaderProgram(const Shader& vertex, const Shader& fragment,
                                 const Shader* geometry, const Shader* tessControl, const Shader* tessEval)
    {
        programID = glCreateProgram();
        if (vertex.GetID()) glAttachShader(programID, vertex.GetID());
        if (fragment.GetID()) glAttachShader(programID, fragment.GetID());
        if (geometry && geometry->GetID()) glAttachShader(programID, geometry->GetID());
        if (tessControl && tessControl->GetID()) glAttachShader(programID, tessControl->GetID());
        if (tessEval && tessEval->GetID()) glAttachShader(programID, tessEval->GetID());

        glLinkProgram(programID);

        try {
            CheckProgramLinkStatus(programID);
        } catch (...) {
            if (vertex.GetID()) glDetachShader(programID, vertex.GetID());
            if (fragment.GetID()) glDetachShader(programID, fragment.GetID());
            if (geometry && geometry->GetID()) glDetachShader(programID, geometry->GetID());
            if (tessControl && tessControl->GetID()) glDetachShader(programID, tessControl->GetID());
            if (tessEval && tessEval->GetID()) glDetachShader(programID, tessEval->GetID());
            glDeleteProgram(programID);
            programID = 0;
            throw;
        }

        m_ownsProgram = true;
    }

    ShaderProgram::~ShaderProgram()
    {
        if (m_ownsProgram && programID != 0)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }

    GLuint ShaderProgram::Release()
    {
        GLuint id = programID;
        m_ownsProgram = false;
        programID = 0;
        return id;
    }

    void ShaderProgram::Bind() const
    {
        if (programID) glUseProgram(programID);
    }

    void ShaderProgram::Unbind() const
    {
        glUseProgram(0);
    }

    void ShaderProgram::SetUniform(const std::string& name, int value) const
    {
        if (!programID) return;
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc >= 0) glUniform1i(loc, value);
    }

    void ShaderProgram::SetUniform(const std::string& name, float value) const
    {
        if (!programID) return;
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc >= 0) glUniform1f(loc, value);
    }

    void ShaderProgram::SetUniform(const std::string& name, const glm::mat4& matrix) const
    {
        if (!programID) return;
        GLint loc = glGetUniformLocation(programID, name.c_str());
        if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &matrix[0][0]);
    }

    GLuint ShaderProgram::GetUniformBlockIndex(const std::string& blockName) const
    {
        if (!programID) return GL_INVALID_INDEX;
        return glGetUniformBlockIndex(programID, blockName.c_str());
    }

    void ShaderProgram::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        if (!programID) return;
        GLuint index = glGetUniformBlockIndex(programID, blockName.c_str());
        if (index != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(programID, index, bindingPoint);
        }
    }

    GLuint ShaderProgram::GetShaderStorageBlockIndex(const std::string& blockName) const
    {
        if (!programID) return GL_INVALID_INDEX;
        return glGetProgramResourceIndex(programID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
    }

    void ShaderProgram::BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        if (!programID) return;
        GLuint index = glGetProgramResourceIndex(programID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
        if (index != GL_INVALID_INDEX) {
            glShaderStorageBlockBinding(programID, index, bindingPoint);
        }
    }

    ComputeShaderProgram::ComputeShaderProgram(const Shader& compute)
    {
        programID = glCreateProgram();
        if (compute.GetID()) glAttachShader(programID, compute.GetID());
        glLinkProgram(programID);
        try {
            CheckProgramLinkStatus(programID);
        } catch (...) {
            if (compute.GetID()) glDetachShader(programID, compute.GetID());
            glDeleteProgram(programID);
            programID = 0;
            throw;
        }
    }

    ComputeShaderProgram::~ComputeShaderProgram()
    {
        if (programID != 0)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }

    void ComputeShaderProgram::Bind() const
    {
        if (programID) glUseProgram(programID);
    }

    void ComputeShaderProgram::Unbind() const
    {
        glUseProgram(0);
    }

    GLuint ComputeShaderProgram::GetUniformBlockIndex(const std::string& blockName) const
    {
        if (!programID) return GL_INVALID_INDEX;
        return glGetUniformBlockIndex(programID, blockName.c_str());
    }

    void ComputeShaderProgram::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        if (!programID) return;
        GLuint index = glGetUniformBlockIndex(programID, blockName.c_str());
        if (index != GL_INVALID_INDEX)
            glUniformBlockBinding(programID, index, bindingPoint);
    }

    GLuint ComputeShaderProgram::GetShaderStorageBlockIndex(const std::string& blockName) const
    {
        if (!programID) return GL_INVALID_INDEX;
        return glGetProgramResourceIndex(programID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
    }

    void ComputeShaderProgram::BindShaderStorageBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        if (!programID) return;
        GLuint index = glGetProgramResourceIndex(programID, GL_SHADER_STORAGE_BLOCK, blockName.c_str());
        if (index != GL_INVALID_INDEX)
            glShaderStorageBlockBinding(programID, index, bindingPoint);
    }

    void ComputeShaderProgram::Dispatch(GLuint x, GLuint y, GLuint z, GLbitfield memoryBarrierBits) const
    {
        if (!programID) return;
        glUseProgram(programID);
        glDispatchCompute(x, y, z);
        if (memoryBarrierBits)
        {
            glMemoryBarrier(memoryBarrierBits);
        }
        glUseProgram(0);
    }

    void ComputeShaderProgram::Dispatch(GLuint x, GLuint y, GLuint z) const
    {
        if (!programID) return;
        glUseProgram(programID);
        glDispatchCompute(x, y, z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        glUseProgram(0);
    }
}

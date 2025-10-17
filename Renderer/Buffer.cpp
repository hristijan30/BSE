#include "Buffer.h"

namespace BSE
{
    UniformBuffer::UniformBuffer(GLsizeiptr size, GLenum usage)
    {
        Create(size, usage);
    }

    UniformBuffer::~UniformBuffer()
    {
        if (bufferID != 0)
        {
            glDeleteBuffers(1, &bufferID);
            bufferID = 0;
            bufferSize = 0;
        }
    }

    void UniformBuffer::Create(GLsizeiptr size, GLenum usage)
    {
        if (bufferID != 0)
        {
            glDeleteBuffers(1, &bufferID);
            bufferID = 0;
            bufferSize = 0;
        }
        bufferUsage = usage;
        bufferSize = size;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
        glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, bufferUsage);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::Update(const void* data, GLsizeiptr size, GLintptr offset) const
    {
        if (!bufferID) return;
        if (offset + size > bufferSize)
        {
            throw std::runtime_error("UniformBuffer::Update - write exceeds buffer size");
        }
        glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::Bind() const
    {
        if (bufferID) glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
    }

    void UniformBuffer::Unbind() const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::BindBase(GLuint bindingPoint) const
    {
        if (!bufferID) return;
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, bufferID);
    }

    GLuint UniformBuffer::Release()
    {
        GLuint id = bufferID;
        bufferID = 0;
        bufferSize = 0;
        return id;
    }

    ShaderStorageBuffer::ShaderStorageBuffer(GLsizeiptr size, GLenum usage)
    {
        Create(size, usage);
    }

    ShaderStorageBuffer::~ShaderStorageBuffer()
    {
        if (bufferID != 0) {
            glDeleteBuffers(1, &bufferID);
            bufferID = 0;
            bufferSize = 0;
        }
    }

    void ShaderStorageBuffer::Create(GLsizeiptr size, GLenum usage)
    {
        if (bufferID != 0) {
            glDeleteBuffers(1, &bufferID);
            bufferID = 0;
            bufferSize = 0;
        }
        bufferUsage = usage;
        bufferSize = size;
        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, bufferUsage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void ShaderStorageBuffer::Update(const void* data, GLsizeiptr size, GLintptr offset) const
    {
        if (!bufferID) return;
        if (offset + size > bufferSize) {
            throw std::runtime_error("ShaderStorageBuffer::Update - write exceeds buffer size");
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void ShaderStorageBuffer::Bind() const
    {
        if (bufferID) glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    }

    void ShaderStorageBuffer::Unbind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void ShaderStorageBuffer::BindBase(GLuint bindingPoint) const
    {
        if (!bufferID) return;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, bufferID);
    }

    GLuint ShaderStorageBuffer::Release()
    {
        GLuint id = bufferID;
        bufferID = 0;
        bufferSize = 0;
        return id;
    }
}

#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include "OpenGL.h"

namespace BSE
{
    class DLL_EXPORT UniformBuffer
    {
    public:
        UniformBuffer() = default;
        UniformBuffer(GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);
        ~UniformBuffer();

        void Create(GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW);

        void Update(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

        void Bind() const;
        void Unbind() const;

        void BindBase(GLuint bindingPoint) const;

        GLuint Release();

        GLuint GetID() const { return bufferID; }
        GLsizeiptr GetSize() const { return bufferSize; }

    private:
        GLuint bufferID = 0;
        GLsizeiptr bufferSize = 0;
        GLenum bufferUsage = GL_DYNAMIC_DRAW;
    };

    class DLL_EXPORT ShaderStorageBuffer
    {
    public:
        ShaderStorageBuffer() = default;
        ShaderStorageBuffer(GLsizeiptr size, GLenum usage = GL_DYNAMIC_COPY);
        ~ShaderStorageBuffer();

        void Create(GLsizeiptr size, GLenum usage = GL_DYNAMIC_COPY);
        void Update(const void* data, GLsizeiptr size, GLintptr offset = 0) const;

        void Bind() const;
        void Unbind() const;

        void BindBase(GLuint bindingPoint) const;

        GLuint Release();

        GLuint GetID() const { return bufferID; }
        GLsizeiptr GetSize() const { return bufferSize; }

    private:
        GLuint bufferID = 0;
        GLsizeiptr bufferSize = 0;
        GLenum bufferUsage = GL_DYNAMIC_COPY;
    };
}

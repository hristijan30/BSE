#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include "../Renderer/OpenGL.h"

class DLL_EXPORT Shape {
public:
    enum class Type {
        Sphere,
        Box,
        Capsule,
        Mesh
    };

    Shape(Type type) : m_type(type) {}
    virtual ~Shape() = default;

    Type getType() const { return m_type; }

    virtual glm::vec3 getSupport(const glm::vec3& dir) const = 0;

    virtual glm::vec3 getAABBMin() const = 0;
    virtual glm::vec3 getAABBMax() const = 0;

protected:
    Type m_type;
};

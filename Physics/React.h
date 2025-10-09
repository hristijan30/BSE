#pragma once

#include "../ThirdParty/reactphysics3d/include/reactphysics3d/reactphysics3d.h"
#include "../ThirdParty/reactphysics3d/include/reactphysics3d/engine/PhysicsWorld.h"
#include "../Renderer/OpenGL.h"

namespace BSE
{
    namespace rp3d = reactphysics3d;

    static inline rp3d::Vector3 glmToRp3d(const glm::vec3& v)
    {
        return rp3d::Vector3(v.x, v.y, v.z);
    }

    static inline glm::vec3 rp3dToGlm(const rp3d::Vector3& v)
    {
        return glm::vec3(v.x, v.y, v.z);
    }

    static inline rp3d::Quaternion glmToRp3d(const glm::quat& q)
    {
        return rp3d::Quaternion(q.w, q.x, q.y, q.z);
    }

    static inline glm::quat rp3dToGlm(const rp3d::Quaternion& q)
    {
        return glm::quat(q.w, q.x, q.y, q.z);
    }
}
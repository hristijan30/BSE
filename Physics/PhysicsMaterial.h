#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "React.h"


namespace BSE
{
    class DLL_EXPORT PhysicsMaterial
    {
    public:
        PhysicsMaterial(float frictionCoefficient = 0.5f, float restitution = 0.0f, float massDensity = 1.0f) noexcept;

        float GetFrictionCoefficient() const { return m_friction; }
        float GetRestitution() const { return m_restitution; }
        float GetMassDensity() const { return m_massDensity; }

        void SetFrictionCoefficient(float f) { m_friction = f; }
        void SetRestitution(float r) { m_restitution = r; }
        void SetMassDensity(float d) { m_massDensity = d; }

        void ApplyToCollider(rp3d::Collider* collider) const;

    private:
        float m_friction;
        float m_restitution;
        float m_massDensity;
    };
}
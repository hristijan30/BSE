#include "PhysicsMaterial.h"

namespace BSE {
    PhysicsMaterial::PhysicsMaterial(float frictionCoefficient, float restitution, float massDensity) noexcept
        : m_friction(frictionCoefficient), m_restitution(restitution), m_massDensity(massDensity)
    {
    }

    void PhysicsMaterial::ApplyToCollider(rp3d::Collider* collider) const {
        if (!collider) return;
        rp3d::Material& mat = collider->getMaterial();
        mat.setFrictionCoefficient(static_cast<rp3d::decimal>(m_friction));
        mat.setBounciness(static_cast<rp3d::decimal>(m_restitution));
        mat.setMassDensity(static_cast<rp3d::decimal>(m_massDensity));
    }
}
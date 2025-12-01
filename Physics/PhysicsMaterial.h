#pragma once

#include "../Engine/StandardInclude.h"

#include "Jolt.h"

namespace BSE
{
    struct PhysicsProperties
    {
        float friction    = 0.5f;
        float restitution = 0.1f;
    };

    class PhysicsMaterial
    {
    public:
        PhysicsMaterial() = default;
        ~PhysicsMaterial() = default;

        void ApplyToBody(JPH::Body& body, const PhysicsProperties& props) const
        {
            body.SetFriction(props.friction);
            body.SetRestitution(props.restitution);
        }

        void ApplyToBodyUsingInterface(JPH::BodyInterface &bodyInterface, const JPH::BodyID &id, const PhysicsProperties& props) const
        {
            if (id.IsInvalid())
                return;
            bodyInterface.SetFriction(id, props.friction);
            bodyInterface.SetRestitution(id, props.restitution);
        }
    };
}

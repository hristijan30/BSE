#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "Jolt.h"

#include "Physics.h"
#include "PhysicsMaterial.h"

namespace BSE
{
    class Model;

    class DLL_EXPORT PhysicsShape
    {
    public:
        PhysicsShape() = default;
        ~PhysicsShape() = default;

        enum class ShapeType
        {
            Box,
            Sphere,
            Capsule
        };

        void CreateBoxCollisionShape(const JPH::Vec3& halfExtents);
        void CreateSphereCollisionShape(float radius);

        JPH::Ref<JPH::Shape> GetJoltShape() const;
        const JPH::Shape* GetJoltShapePtr() const;

    private:
        ShapeType m_type = ShapeType::Box;
        JPH::Ref<JPH::Shape> m_shape;
    };

    class DLL_EXPORT PhysicsBody
    {
    public:
        PhysicsBody() = default;
        ~PhysicsBody() = default;

        void CreateBody(const PhysicsShape& shape,
                        const PhysicsMaterial& material,
                        const PhysicsProperties& properties,
                        JPH::BodyCreationSettings settings,
                        JPH::BodyInterface &bodyInterface);

        void CreateBody_CreateAndAdd(const PhysicsShape& shape,
                                     const PhysicsMaterial& material,
                                     const PhysicsProperties& properties,
                                     JPH::BodyCreationSettings settings,
                                     JPH::BodyInterface &bodyInterface);

        void UpdateMaterial(const PhysicsMaterial& material, const PhysicsProperties& properties, JPH::BodyInterface &bodyInterface);

        void RemoveFromSimulation(JPH::BodyInterface &bodyInterface);

        JPH::BodyID GetBodyID() const;
        std::shared_ptr<PhysicsShape> GetShape() const;
        std::shared_ptr<PhysicsMaterial> GetMaterial() const;

        void AttachModel(BSE::Model* model);
        void DetachModel();

        void SyncModelFromPhysics(JPH::BodyInterface &bodyInterface);
        void SyncPhysicsFromModel(JPH::BodyInterface &bodyInterface, float tick);

    private:
        std::shared_ptr<PhysicsShape> m_shape;
        std::shared_ptr<PhysicsMaterial> m_material;
        PhysicsProperties m_properties;

        JPH::BodyID m_bodyID = JPH::BodyID();

        BSE::Model* m_model = nullptr;
    };
}

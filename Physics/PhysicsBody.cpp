#include "PhysicsBody.h"
#include "../Renderer/Model.h"

namespace BSE
{
    void PhysicsShape::CreateBoxCollisionShape(const JPH::Vec3& halfExtents)
    {
        m_type = ShapeType::Box;
        m_shape = new JPH::BoxShape(halfExtents);
    }

    void PhysicsShape::CreateSphereCollisionShape(float radius)
    {
        m_type = ShapeType::Sphere;
        m_shape = new JPH::SphereShape(radius);
    }

    JPH::Ref<JPH::Shape> PhysicsShape::GetJoltShape() const
    {
        return m_shape;
    }

    const JPH::Shape* PhysicsShape::GetJoltShapePtr() const
    {
        return m_shape.GetPtr();
    }

    void PhysicsBody::CreateBody(const PhysicsShape& shape,
                                 const PhysicsMaterial& material,
                                 const PhysicsProperties& properties,
                                 JPH::BodyCreationSettings settings,
                                 JPH::BodyInterface &bodyInterface)
    {
        m_shape = std::make_shared<PhysicsShape>(shape);
        m_material = std::make_shared<PhysicsMaterial>(material);
        m_properties = properties;

        settings.SetShape(m_shape->GetJoltShapePtr());

        JPH::Body* body = bodyInterface.CreateBody(settings);
        if (body == nullptr)
        {
            m_bodyID = JPH::BodyID();
            return;
        }

        m_bodyID = body->GetID();

        m_material->ApplyToBody(*body, m_properties);

        bodyInterface.AddBody(m_bodyID, JPH::EActivation::Activate);
    }

    void PhysicsBody::CreateBody_CreateAndAdd(const PhysicsShape& shape,
                                              const PhysicsMaterial& material,
                                              const PhysicsProperties& properties,
                                              JPH::BodyCreationSettings settings,
                                              JPH::BodyInterface &bodyInterface)
    {
        m_shape = std::make_shared<PhysicsShape>(shape);
        m_material = std::make_shared<PhysicsMaterial>(material);
        m_properties = properties;

        settings.SetShape(m_shape->GetJoltShapePtr());

        JPH::BodyID id = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
        if (id.IsInvalid())
        {
            m_bodyID = JPH::BodyID();
            return;
        }

        m_bodyID = id;

        bodyInterface.SetFriction(m_bodyID, m_properties.friction);
        bodyInterface.SetRestitution(m_bodyID, m_properties.restitution);
    }

    void PhysicsBody::UpdateMaterial(const PhysicsMaterial& material, const PhysicsProperties& properties, JPH::BodyInterface &bodyInterface)
    {
        m_material = std::make_shared<PhysicsMaterial>(material);
        m_properties = properties;

        if (!m_bodyID.IsInvalid())
        {
            bodyInterface.SetFriction(m_bodyID, m_properties.friction);
            bodyInterface.SetRestitution(m_bodyID, m_properties.restitution);
        }
    }

    void PhysicsBody::RemoveFromSimulation(JPH::BodyInterface &bodyInterface)
    {
        if (!m_bodyID.IsInvalid())
        {
            bodyInterface.RemoveBody(m_bodyID);
            m_bodyID = JPH::BodyID();
        }
    }

    JPH::BodyID PhysicsBody::GetBodyID() const
    {
        return m_bodyID;
    }

    std::shared_ptr<PhysicsShape> PhysicsBody::GetShape() const
    {
        return m_shape;
    }

    std::shared_ptr<PhysicsMaterial> PhysicsBody::GetMaterial() const
    {
        return m_material;
    }

    void PhysicsBody::AttachModel(BSE::Model* model)
    {
        m_model = model;
    }

    void PhysicsBody::DetachModel()
    {
        m_model = nullptr;
    }

    void PhysicsBody::SyncModelFromPhysics(JPH::BodyInterface &bodyInterface)
    {
        if (m_model == nullptr || m_bodyID.IsInvalid())
            return;

        glm::vec3 pos  = Physics::FromJolt(bodyInterface.GetPosition(m_bodyID));
        glm::quat rot  = Physics::FromJolt(bodyInterface.GetRotation(m_bodyID));

        m_model->SetPosition(pos);
        m_model->SetRotation(rot);
    }

    void PhysicsBody::SyncPhysicsFromModel(JPH::BodyInterface &bodyInterface, float tick)
    {
        if (m_model == nullptr || m_bodyID.IsInvalid())
            return;

        JPH::Vec3 jpos = Physics::ToJolt(m_model->GetPosition());
        JPH::Quat jrot = Physics::ToJolt(m_model->GetRotation());

        bodyInterface.MoveKinematic(m_bodyID, jpos, jrot, tick);
    }
}
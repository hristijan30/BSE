#include "PhysicsBody.h"

namespace BSE
{
    PhysicsBody::PhysicsBody(rp3d::PhysicsWorld* world, rp3d::PhysicsCommon& physicsCommon, rp3d::BodyType bodyType)
        : m_world(world), m_physicsCommon(&physicsCommon)
    {
        rp3d::Transform t = rp3d::Transform::identity();
        m_body = m_world->createRigidBody(t);
        m_body->setType(bodyType);
    }

    PhysicsBody::~PhysicsBody()
    {
        if (m_world && m_body)
        {
            m_world->destroyRigidBody(m_body);
            m_body = nullptr;
        }

        if (m_physicsCommon)
        {
            for (rp3d::CollisionShape* shape : m_collisionShapes)
            {
                if (!shape) continue;
                if (auto concave = dynamic_cast<rp3d::ConcaveMeshShape*>(shape))
                {
                    m_physicsCommon->destroyConcaveMeshShape(concave);
                }
            }
        }

        for (rp3d::TriangleVertexArray* tva : m_triangleVertexArrays) delete tva;
        for (rp3d::TriangleMesh* tm : m_triangleMeshes) delete tm;
    }

    void PhysicsBody::SetTransformFromModel(const Model& model)
    {
        glm::vec3 pos = model.GetPosition();
        glm::quat rot  = model.GetRotation();
        rp3d::Transform t(BSE::glmToRp3d(pos), BSE::glmToRp3d(rot));
        if (m_body) m_body->setTransform(t);
    }

    void PhysicsBody::SyncModelTransform(Model& model) const {
        if (!m_body) return;
        const rp3d::Transform& t = m_body->getTransform();
        model.SetPosition(rp3dToGlm(t.getPosition()));
        model.SetRotation(rp3dToGlm(t.getOrientation()));
    }
}
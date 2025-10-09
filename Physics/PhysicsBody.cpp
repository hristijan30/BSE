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
                else if (auto convexMesh = dynamic_cast<rp3d::ConvexMeshShape*>(shape))
                {
                    m_physicsCommon->destroyConvexMeshShape(convexMesh);
                }
                else if (auto box = dynamic_cast<rp3d::BoxShape*>(shape))
                {
                    m_physicsCommon->destroyBoxShape(box);
                }
                else if (auto sphere = dynamic_cast<rp3d::SphereShape*>(shape))
                {
                    m_physicsCommon->destroySphereShape(sphere);
                }
                else if (auto capsule = dynamic_cast<rp3d::CapsuleShape*>(shape))
                {
                    m_physicsCommon->destroyCapsuleShape(capsule);
                }
            }
            m_collisionShapes.clear();

            for (rp3d::TriangleMesh* tm : m_triangleMeshes) {
                if (tm) {
                    m_physicsCommon->destroyTriangleMesh(tm);
                }
            }
            m_triangleMeshes.clear();
        }

        m_ownedVertexBuffers.clear();
        m_ownedIndexBuffers.clear();

        for (rp3d::TriangleVertexArray* tva : m_triangleVertexArrays)
        {
            delete tva;
        }
        m_triangleVertexArrays.clear();
    }

    rp3d::Collider* PhysicsBody::AddConcaveMeshCollider(const MeshData& mesh, const PhysicsMaterial* material, const glm::mat4& localTransform)
    {
        if (!m_physicsCommon || !m_body) return nullptr;

        const size_t vertexCount = mesh.positions.size();
        const size_t indexCount  = mesh.indices.size();
        if (vertexCount == 0 || indexCount < 3) return nullptr;

        const size_t triangleCount = indexCount / 3;
        if (triangleCount == 0) return nullptr;

        std::vector<float> verts;
        verts.reserve(vertexCount * 3);
        for (const auto& v : mesh.positions)
        {
            verts.push_back(v.x);
            verts.push_back(v.y);
            verts.push_back(v.z);
        }

        std::vector<int> inds;
        inds.reserve(indexCount);
        for (uint32_t i : mesh.indices) inds.push_back(static_cast<int>(i));

        rp3d::TriangleVertexArray triangleArray(
            static_cast<rp3d::uint32>(vertexCount),
            verts.data(),
            static_cast<rp3d::uint32>(3 * sizeof(float)),
            static_cast<rp3d::uint32>(triangleCount),
            inds.data(),
            static_cast<rp3d::uint32>(3 * sizeof(int)),
            rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
        );

        std::vector<rp3d::Message> messages;
        rp3d::TriangleMesh* triMesh = m_physicsCommon->createTriangleMesh(triangleArray, messages);

        if (!messages.empty())
        {
            for (const auto& msg : messages)
            {
                std::cerr << msg.text << std::endl;
            }
        }

        if (!triMesh)
        {
            return nullptr;
        }

        m_triangleMeshes.push_back(triMesh);

        rp3d::ConcaveMeshShape* concaveShape = m_physicsCommon->createConcaveMeshShape(triMesh);
        if (!concaveShape)
        {
            m_physicsCommon->destroyTriangleMesh(triMesh);
            m_triangleMeshes.pop_back();
            return nullptr;
        }

        m_collisionShapes.push_back(concaveShape);

        glm::vec3 localPos = glm::vec3(localTransform[3]);
        glm::quat localRot = glm::quat_cast(localTransform);
        rp3d::Transform colliderTransform(BSE::glmToRp3d(localPos), BSE::glmToRp3d(localRot));

        rp3d::Collider* collider = m_body->addCollider(concaveShape, colliderTransform);

        if (material && collider)
        {
            material->ApplyToCollider(collider);
        }

        return collider;
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
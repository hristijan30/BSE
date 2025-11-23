#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "../Renderer/Model.h"

#include "React.h"
#include "PhysicsMaterial.h"

namespace BSE
{
    using PhysicsWorld = rp3d::PhysicsWorld;
    using PhysicsCommon = rp3d::PhysicsCommon;
    using BodyType = rp3d::BodyType;
    using Collider = rp3d::Collider;
    using RigidBody = rp3d::RigidBody;

    class DLL_EXPORT PhysicsBody
    {
    public:
        PhysicsBody(PhysicsWorld* world, PhysicsCommon& physicsCommon, BodyType bodyType = BodyType::DYNAMIC);
        ~PhysicsBody();

        Collider* AddConcaveMeshCollider(const MeshData& mesh, const PhysicsMaterial* material = nullptr, const glm::mat4& localTransform = glm::mat4(1.0f));

        void SetTransformFromModel(const Model& model);
        void SyncModelTransform(Model& model) const;

        RigidBody* GetRigidBody() const { return m_body; }

    private:
        PhysicsWorld* m_world = nullptr;
        PhysicsCommon* m_physicsCommon = nullptr;
        RigidBody* m_body = nullptr;

        std::vector<rp3d::TriangleVertexArray*> m_triangleVertexArrays;
        std::vector<rp3d::TriangleMesh*> m_triangleMeshes;
        std::vector<rp3d::CollisionShape*> m_collisionShapes;
        std::vector<std::vector<float>> m_ownedVertexBuffers;
        std::vector<std::vector<int>> m_ownedIndexBuffers;
    };
}

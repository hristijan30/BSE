#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "../Renderer/Model.h"

#include "React.h"
#include "PhysicsMaterial.h"

namespace BSE
{
    class DLL_EXPORT PhysicsBody
    {
    public:
        PhysicsBody(rp3d::PhysicsWorld* world, rp3d::PhysicsCommon& physicsCommon, rp3d::BodyType bodyType = rp3d::BodyType::DYNAMIC);
        ~PhysicsBody();

        rp3d::Collider* AddConcaveMeshCollider(const MeshData& mesh, const PhysicsMaterial* material = nullptr, const glm::mat4& localTransform = glm::mat4(1.0f));

        void SetTransformFromModel(const Model& model);
        void SyncModelTransform(Model& model) const;

        rp3d::RigidBody* GetRigidBody() const { return m_body; }

    private:
        rp3d::PhysicsWorld* m_world = nullptr;
        rp3d::PhysicsCommon* m_physicsCommon = nullptr;
        rp3d::RigidBody* m_body = nullptr;

        std::vector<rp3d::TriangleVertexArray*> m_triangleVertexArrays;
        std::vector<rp3d::TriangleMesh*> m_triangleMeshes;
        std::vector<rp3d::CollisionShape*> m_collisionShapes;
        std::vector<std::vector<float>> m_ownedVertexBuffers;
        std::vector<std::vector<int>> m_ownedIndexBuffers;
    };
}

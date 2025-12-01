#include "Physics.h"
#include "../Engine/Engine.h"

namespace BSE
{
    enum class MyObjectLayer : uint32_t
    {
        STATIC = 0,
        DYNAMIC = 1,
        LAYER_COUNT
    };

    struct SimpleBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
    {
        SimpleBroadPhaseLayerInterface(uint32_t inNumLayers) : mNumLayers(inNumLayers) {}

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            return JPH::BroadPhaseLayer(static_cast<uint32_t>(inLayer));
        }

        virtual uint32_t GetNumBroadPhaseLayers() const override
        {
            return mNumLayers;
        }

        virtual const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer) const override
        {
            return "BroadPhaseLayer";
        }

    private:
        uint32_t mNumLayers;
    };

    struct SimpleObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
        virtual bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override
        {
            return true;
        }
    };

    struct SimpleObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
        virtual bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override
        {
            return true;
        }
    };

    void PhysicsCore::Initialize()
    {
        unsigned int cores = Engine::GetCPUThreadCount();
        unsigned int physicsThreads = std::clamp(cores / 2, 2u, 4u);

        m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(1024, 256, physicsThreads);
        m_JobSystem->Init(1024, 256, physicsThreads);

        JPH::RegisterDefaultAllocator();

        m_PhysicsSystem = std::make_unique<JPH::PhysicsSystem>();

        const uint32_t numObjectLayers = static_cast<uint32_t>(MyObjectLayer::LAYER_COUNT);
        const uint32_t numBroadPhaseLayers = numObjectLayers;

        m_BroadPhaseLayerInterface = std::make_unique<SimpleBroadPhaseLayerInterface>(numBroadPhaseLayers);
        m_ObjectLayerPairFilter = std::make_unique<SimpleObjectLayerPairFilter>();
        m_ObjectVsBroadPhaseLayerFilter = std::make_unique<SimpleObjectVsBroadPhaseLayerFilter>();

        m_PhysicsSystem->Init(
            2000,
            512,
            1000,
            2000,
            *m_BroadPhaseLayerInterface,
            *m_ObjectVsBroadPhaseLayerFilter,
            *m_ObjectLayerPairFilter
        );
    }

    void PhysicsCore::Destroy()
    {
        m_PhysicsSystem.reset();

        m_JobSystem.reset();
        m_TempAllocator.reset();

        m_ObjectVsBroadPhaseLayerFilter.reset();
        m_ObjectLayerPairFilter.reset();
        m_BroadPhaseLayerInterface.reset();
    }
}

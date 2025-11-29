#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

#include "Jolt.h"

namespace BSE
{
    class DLL_EXPORT PhysicsCore
    {
    public:
        void Initialize();
        void Destroy();

        JPH::PhysicsSystem& GetPhysicsSystem() { return *m_PhysicsSystem; }

    private:
        std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
        std::unique_ptr<JPH::PhysicsSystem> m_PhysicsSystem;
        
        std::unique_ptr<JPH::BroadPhaseLayerInterface> m_BroadPhaseLayerInterface;
        std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter> m_ObjectVsBroadPhaseLayerFilter;
        std::unique_ptr<JPH::ObjectLayerPairFilter> m_ObjectLayerPairFilter;
    };
}
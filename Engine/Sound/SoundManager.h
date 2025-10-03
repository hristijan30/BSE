#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include "../Renderer/OpenGL.h"
#include "OpenAL.h"

namespace BSE
{
    class DLL_EXPORT SoundManager
    {
    public:
        SoundManager();
        ~SoundManager();

        bool Initialize();
        void Shutdown();

        void SetListenerPosition(const glm::vec3& pos);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
        void SetListenerVelocity(const glm::vec3& vel);

    private:
        ALCdevice* m_device = nullptr;
        ALCcontext* m_context = nullptr;
    };
}

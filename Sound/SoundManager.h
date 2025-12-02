#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"

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

    private:
        ALCdevice* m_device = nullptr;
        ALCcontext* m_context = nullptr;
    };
}

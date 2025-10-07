#include "SoundManager.h"

namespace BSE
{
    SoundManager::SoundManager()
        : m_device(nullptr), m_context(nullptr)
    {
    }

    SoundManager::~SoundManager()
    {
        Shutdown();
    }

    bool SoundManager::Initialize()
    {
        m_device = alcOpenDevice(nullptr);
        if (!m_device)
        {
            std::cerr << "[SoundManager] Failed to open audio device.\n";
            return false;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context || !alcMakeContextCurrent(m_context))
        {
            std::cerr << "[SoundManager] Failed to create/make current context.\n";
            if (m_context) alcDestroyContext(m_context);
            if (m_device) alcCloseDevice(m_device);
            return false;
        }

        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        float ori[6] = { 0.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f };
        alListenerfv(AL_ORIENTATION, ori);

        std::cout << "[SoundManager] OpenAL initialized successfully.\n";
        return true;
    }

    void SoundManager::Shutdown()
    {
        if (m_context)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(m_context);
            m_context = nullptr;
        }
        if (m_device)
        {
            alcCloseDevice(m_device);
            m_device = nullptr;
        }
    }

    void SoundManager::SetListenerPosition(const glm::vec3& pos)
    {
        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    }

    void SoundManager::SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up)
    {
        float ori[6] = { forward.x, forward.y, forward.z,  up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, ori);
    }

    void SoundManager::SetListenerVelocity(const glm::vec3& vel)
    {
        alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
    }
}

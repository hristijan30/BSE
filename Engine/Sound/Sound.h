#pragma once

#include "../Define.h"
#include "../StandardInclude.h"

#include "../Renderer/OpenGL.h"
#include "OpenAL.h"

namespace BSE
{
    class DLL_EXPORT SoundBuffer
    {
    public:
        SoundBuffer();
        ~SoundBuffer();

        bool LoadFromFile(const std::string& filepath);
        void SetData(const void* data, ALsizei size, ALsizei freq, ALenum format);

        ALuint GetID() const { return m_buffer; }

    private:
        ALuint m_buffer = 0;
    };

    class DLL_EXPORT SoundSource
    {
    public:
        SoundSource();
        ~SoundSource();

        void AttachBuffer(const SoundBuffer& buffer);

        void Play();
        void Pause();
        void Stop();

        void SetLooping(bool loop);
        void SetGain(float gain);
        void SetPitch(float pitch);
        void SetPosition(const glm::vec3& pos);
        void SetVelocity(const glm::vec3& vel);

        bool IsPlaying() const;

        ALuint GetID() const { return m_source; }

    private:
        ALuint m_source = 0;
    };
}

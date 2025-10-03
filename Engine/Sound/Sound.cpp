#include "Sound.h"

namespace BSE
{
    SoundBuffer::SoundBuffer()
    {
        alGenBuffers(1, &m_buffer);
    }

    SoundBuffer::~SoundBuffer()
    {
        if (m_buffer)
            alDeleteBuffers(1, &m_buffer);
    }

    bool SoundBuffer::LoadFromFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        if (!file)
        {
            std::cerr << "[SoundBuffer] Failed to open file: " << filepath << "\n";
            return false;
        }

        char riff[4];
        file.read(riff, 4);
        if (std::string(riff, 4) != "RIFF")
        {
            std::cerr << "[SoundBuffer] Not a RIFF file.\n";
            return false;
        }

        file.ignore(4);
        char wave[4];
        file.read(wave, 4);
        if (std::string(wave, 4) != "WAVE")
        {
            std::cerr << "[SoundBuffer] Not a WAVE file.\n";
            return false;
        }

        char chunkId[4];
        int32_t chunkSize;
        short audioFormat = 0, numChannels = 0, bitsPerSample = 0;
        int32_t sampleRate = 0;
        std::vector<char> data;

        while (file.read(chunkId, 4))
        {
            file.read(reinterpret_cast<char*>(&chunkSize), 4);

            if (std::string(chunkId, 4) == "fmt ")
            {
                file.read(reinterpret_cast<char*>(&audioFormat), 2);
                file.read(reinterpret_cast<char*>(&numChannels), 2);
                file.read(reinterpret_cast<char*>(&sampleRate), 4);
                file.ignore(6);
                file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

                if (chunkSize > 16)
                    file.ignore(chunkSize - 16);
            }
            else if (std::string(chunkId, 4) == "data")
            {
                data.resize(chunkSize);
                file.read(data.data(), chunkSize);
                break;
            }
            else
            {
                file.ignore(chunkSize);
            }
        }

        if (audioFormat != 1)
        {
            std::cerr << "[SoundBuffer] Unsupported WAV format (only PCM supported).\n";
            return false;
        }

        ALenum format = 0;
        if (numChannels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
        else if (numChannels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
        else if (numChannels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
        else if (numChannels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;
        else
        {
            std::cerr << "[SoundBuffer] Unsupported channel/bit depth combination.\n";
            return false;
        }

        alBufferData(m_buffer, format, data.data(), static_cast<ALsizei>(data.size()), sampleRate);

        return true;
    }

    void SoundBuffer::SetData(const void* data, ALsizei size, ALsizei freq, ALenum format)
    {
        alBufferData(m_buffer, format, data, size, freq);
    }

    SoundSource::SoundSource()
    {
        alGenSources(1, &m_source);
    }

    SoundSource::~SoundSource()
    {
        if (m_source)
            alDeleteSources(1, &m_source);
    }

    void SoundSource::AttachBuffer(const SoundBuffer& buffer)
    {
        alSourcei(m_source, AL_BUFFER, buffer.GetID());
    }

    void SoundSource::Play()  { alSourcePlay(m_source); }
    void SoundSource::Pause() { alSourcePause(m_source); }
    void SoundSource::Stop()  { alSourceStop(m_source); }

    void SoundSource::SetLooping(bool loop)
    {
        alSourcei(m_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    }

    void SoundSource::SetGain(float gain)
    {
        alSourcef(m_source, AL_GAIN, gain);
    }

    void SoundSource::SetPitch(float pitch)
    {
        alSourcef(m_source, AL_PITCH, pitch);
    }

    void SoundSource::SetPosition(const glm::vec3& pos)
    {
        alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    }

    void SoundSource::SetVelocity(const glm::vec3& vel)
    {
        alSource3f(m_source, AL_VELOCITY, vel.x, vel.y, vel.z);
    }

    bool SoundSource::IsPlaying() const
    {
        ALint state;
        alGetSourcei(m_source, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }
}

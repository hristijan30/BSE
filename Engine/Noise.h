#pragma once

#include "Define.h"
#include "StandardInclude.h"

#include "../Renderer/Texture2D.h"

namespace BSE
{
    enum class NoiseType
    {
        White,
        Value,
        Perlin,
        Simplex,
        Worley,
        FBM
    };

    struct DLL_EXPORT NoiseSettings
    {
        float scale = 1.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        int octaves = 4;
        float persistence = 0.5f;
        float lacunarity = 2.0f;
        float time = 0.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        bool normalize = true;
    };

    class DLL_EXPORT SimpleNoise
    {
    public:
        explicit SimpleNoise(const std::string& dna = "");

        void SetDNA(const std::string& dna);
        const std::string& GetDNA() const { return m_dna; }

        void SetSettings(const NoiseSettings& s) { m_settings = s; }
        NoiseSettings GetSettings() const { return m_settings; }
        void SetScale(float s) { m_settings.scale = s; }
        void SetAmplitude(float a) { m_settings.amplitude = a; }
        void SetFrequency(float f) { m_settings.frequency = f; }
        void SetOctaves(int o) { m_settings.octaves = o; }
        void SetPersistence(float p) { m_settings.persistence = p; }
        void SetLacunarity(float l) { m_settings.lacunarity = l; }
        void SetTime(float t) { m_settings.time = t; }
        void SetOffset(float ox, float oy) { m_settings.offsetX = ox; m_settings.offsetY = oy; }
        void SetNormalize(bool n) { m_settings.normalize = n; }

        std::vector<float> GenerateNoiseMap(int width, int height, NoiseType type);

        Texture2D GenerateTexture(int width, int height, NoiseType type, int channels = 3);

        float WhiteNoise(int x, int y);
        float ValueNoise(float x, float y);
        float PerlinNoise(float x, float y);
        float SimplexNoise(float x, float y);
        float WorleyNoise(float x, float y);
        float FBMNoise(float x, float y);

    private:
        std::string m_dna;
        uint64_t m_seed = 0;
        std::mt19937_64 m_rng;

        NoiseSettings m_settings;

        std::array<int, 512> m_perm;

        void InitializeFromDNA(const std::string& dna);
        void InitPermutation();

        uint64_t HashCoords(int x, int y, uint64_t salt = 1469598103934665603ULL) const;

        // Perlin helpers
        static float Fade(float t);
        static float Lerp(float a, float b, float t);
        static float Grad(int hash, float x, float y);

        static const float F2;
        static const float G2;

        void NormalizeMap(std::vector<float>& map);

        static inline float Sat(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
    };
}
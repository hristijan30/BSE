#include "Noise.h"

namespace BSE
{
    const float SimpleNoise::F2 = 0.5f * (std::sqrt(3.0f) - 1.0f);
    const float SimpleNoise::G2 = (3.0f - std::sqrt(3.0f)) / 6.0f;

    SimpleNoise::SimpleNoise(const std::string& dna)
    {
        SetDNA(dna);
    }

    void SimpleNoise::SetDNA(const std::string& dna)
    {
        m_dna = dna;
        InitializeFromDNA(dna);
        InitPermutation();
    }

    void SimpleNoise::InitializeFromDNA(const std::string& dna)
    {
        std::string d = dna;
        if (d.empty())
        {
            std::random_device rd;
            uint64_t r = ((uint64_t)rd() << 32) ^ rd();
            d = std::to_string(r);
        }

        std::hash<std::string> hasher;
        m_seed = static_cast<uint64_t>(hasher(d));
        m_rng.seed(m_seed);

        auto pick = [&](int start, int len) -> uint64_t {
            if (start >= (int)d.size()) start = start % (int)d.size();
            int l = std::min(len, (int)d.size());
            std::string sub = d.substr(start, l);
            return static_cast<uint64_t>(hasher(sub));
        };

        float scale = 0.1f + (pick(0, 8) % 8000) / 1000.0f; // up to 8.1
        float amplitude = 0.25f + (pick(8, 8) % 2250) / 1000.0f;
        float frequency = 0.1f + (pick(16, 8) % 8000) / 1000.0f;
        int octaves = int(pick(24, 4) % 8) + 1;
        float persistence = 0.1f + (pick(28, 4) % 80) / 100.0f;
        float lacunarity = 1.2f + (pick(32, 4) % 180) / 100.0f;
        float time = float(pick(36, 8) % 1000) / 10.0f;

        float ox = float(pick(44, 6) % 10000) / 100.0f;
        float oy = float(pick(50, 6) % 10000) / 100.0f;

        m_settings.scale = scale;
        m_settings.amplitude = amplitude;
        m_settings.frequency = frequency;
        m_settings.octaves = octaves;
        m_settings.persistence = persistence;
        m_settings.lacunarity = lacunarity;
        m_settings.time = time;
        m_settings.offsetX = ox;
        m_settings.offsetY = oy;
        m_settings.normalize = true;
    }

    void SimpleNoise::InitPermutation()
    {
        std::array<int, 256> p;
        for (int i = 0; i < 256; ++i) p[i] = i;

        std::mt19937_64 eng(m_seed);
        for (int i = 255; i > 0; --i)
        {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(eng);
            std::swap(p[i], p[j]);
        }

        for (int i = 0; i < 256; ++i)
        {
            m_perm[i] = p[i];
            m_perm[i + 256] = p[i];
        }
    }

    uint64_t SimpleNoise::HashCoords(int x, int y, uint64_t salt) const
    {
        uint64_t h = m_seed ^ static_cast<uint64_t>(x) * 0x9e3779b97f4a7c15ULL;
        h ^= static_cast<uint64_t>(y) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        h ^= salt;
        h += 0x9e3779b97f4a7c15ULL;
        h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ULL;
        h = (h ^ (h >> 27)) * 0x94d049bb133111ebULL;
        h = h ^ (h >> 31);
        return h;
    }

    std::vector<float> SimpleNoise::GenerateNoiseMap(int width, int height, NoiseType type)
    {
        assert(width > 0 && height > 0);
        std::vector<float> map;
        map.resize(width * height);

        const auto s = m_settings;
        const float invW = 1.0f / std::max(1, width);
        const float invH = 1.0f / std::max(1, height);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                float u = (x + 0.5f) * invW;
                float v = (y + 0.5f) * invH;

                float px = (u * s.scale * s.frequency) + s.offsetX;
                float py = (v * s.scale * s.frequency) + s.offsetY;

                px += s.time * 0.01f;
                py += s.time * 0.01f;

                float value = 0.0f;
                switch (type)
                {
                case NoiseType::White:
                    value = WhiteNoise(x, y);
                    break;
                case NoiseType::Value:
                    value = ValueNoise(px, py);
                    break;
                case NoiseType::Perlin:
                    value = PerlinNoise(px, py);
                    break;
                case NoiseType::Simplex:
                    value = SimplexNoise(px, py);
                    break;
                case NoiseType::Worley:
                    value = WorleyNoise(px, py);
                    break;
                case NoiseType::FBM:
                    value = FBMNoise(px, py);
                    break;
                default:
                    value = PerlinNoise(px, py);
                    break;
                }

                value *= s.amplitude;

                map[y * width + x] = value;
            }
        }

        if (m_settings.normalize)
            NormalizeMap(map);

        for (auto &v : map) v = Sat(v);

        return map;
    }

    Texture2D SimpleNoise::GenerateTexture(int width, int height, NoiseType type, int channels)
    {
        auto map = GenerateNoiseMap(width, height, type);

        ImageData data;
        data.width = width;
        data.height = height;
        data.channels = channels;

        if (channels == 1)
        {
            data.pixels.resize(width * height);
            for (int i = 0; i < width * height; ++i)
            {
                unsigned char c = static_cast<unsigned char>(std::round(map[i] * 255.0f));
                data.pixels[i] = c;
            }
        }
        else
        {
            data.channels = 3;
            data.pixels.resize(width * height * 3);
            for (int i = 0; i < width * height; ++i)
            {
                unsigned char c = static_cast<unsigned char>(std::round(map[i] * 255.0f));
                data.pixels[i * 3 + 0] = c;
                data.pixels[i * 3 + 1] = c;
                data.pixels[i * 3 + 2] = c;
            }
        }

        Texture2D tex;
        tex.CreateFromImageData(data, true);
        return tex;
    }

    float SimpleNoise::WhiteNoise(int x, int y)
    {
        uint64_t h = HashCoords(x, y);
        // normalize to 0..1
        return float((h & 0xffffffffULL) / float(0xffffffffULL));
    }

    float SimpleNoise::ValueNoise(float x, float y)
    {
        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));
        float xf = x - xi;
        float yf = y - yi;

        auto corner = [&](int cx, int cy) -> float {
            uint64_t h = HashCoords(cx, cy);
            return float((h & 0xffffffffULL) / float(0xffffffffULL));
        };

        float v00 = corner(xi, yi);
        float v10 = corner(xi + 1, yi);
        float v01 = corner(xi, yi + 1);
        float v11 = corner(xi + 1, yi + 1);

        float u = Fade(xf);
        float v = Fade(yf);

        float x1 = Lerp(v00, v10, u);
        float x2 = Lerp(v01, v11, u);
        float res = Lerp(x1, x2, v);
        return res;
    }

    float SimpleNoise::PerlinNoise(float x, float y)
    {
        // coordinates for unit grid cell
        int xi = static_cast<int>(std::floor(x)) & 255;
        int yi = static_cast<int>(std::floor(y)) & 255;

        float xf = x - std::floor(x);
        float yf = y - std::floor(y);

        float u = Fade(xf);
        float v = Fade(yf);

        int aa = m_perm[xi + m_perm[yi]];
        int ab = m_perm[xi + m_perm[yi + 1]];
        int ba = m_perm[xi + 1 + m_perm[yi]];
        int bb = m_perm[xi + 1 + m_perm[yi + 1]];

        float x1 = Lerp(Grad(aa, xf, yf), Grad(ba, xf - 1.0f, yf), u);
        float x2 = Lerp(Grad(ab, xf, yf - 1.0f), Grad(bb, xf - 1.0f, yf - 1.0f), u);

        float res = (Lerp(x1, x2, v) + 1.0f) * 0.5f;
        return res;
    }

    float SimpleNoise::SimplexNoise(float xin, float yin)
    {
        float s = (xin + yin) * F2;
        int i = static_cast<int>(std::floor(xin + s));
        int j = static_cast<int>(std::floor(yin + s));
        float t = (i + j) * G2;
        float X0 = i - t;
        float Y0 = j - t;
        float x0 = xin - X0;
        float y0 = yin - Y0;

        int i1, j1;
        if (x0 > y0) { i1 = 1; j1 = 0; }
        else { i1 = 0; j1 = 1; }

        float x1 = x0 - i1 + G2;
        float y1 = y0 - j1 + G2;
        float x2 = x0 - 1.0f + 2.0f * G2;
        float y2 = y0 - 1.0f + 2.0f * G2;

        int ii = i & 255;
        int jj = j & 255;
        int gi0 = m_perm[ii + m_perm[jj]] % 12;
        int gi1 = m_perm[ii + i1 + m_perm[jj + j1]] % 12;
        int gi2 = m_perm[ii + 1 + m_perm[jj + 1]] % 12;

        static const int grad3[12][3] = {
            {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
            {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
            {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
        };

        float t0 = 0.5f - x0*x0 - y0*y0;
        float n0 = 0.0f;
        if (t0 >= 0.0f) {
            t0 *= t0;
            n0 = t0 * t0 * (grad3[gi0][0] * x0 + grad3[gi0][1] * y0);
        }

        float t1 = 0.5f - x1*x1 - y1*y1;
        float n1 = 0.0f;
        if (t1 >= 0.0f) {
            t1 *= t1;
            n1 = t1 * t1 * (grad3[gi1][0] * x1 + grad3[gi1][1] * y1);
        }

        float t2 = 0.5f - x2*x2 - y2*y2;
        float n2 = 0.0f;
        if (t2 >= 0.0f) {
            t2 *= t2;
            n2 = t2 * t2 * (grad3[gi2][0] * x2 + grad3[gi2][1] * y2);
        }

        float result = 70.0f * (n0 + n1 + n2);
        result = (result + 1.0f) * 0.5f;
        return result;
    }

    float SimpleNoise::WorleyNoise(float x, float y)
    {
        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));

        float best = std::numeric_limits<float>::infinity();
        for (int oy = -1; oy <= 1; ++oy)
        {
            for (int ox = -1; ox <= 1; ++ox)
            {
                int cx = xi + ox;
                int cy = yi + oy;

                uint64_t h = HashCoords(cx, cy);
                float fx = float((h & 0xffff) / float(0xffff)); // 0..1
                float fy = float(((h >> 16) & 0xffff) / float(0xffff));
                float px = cx + fx;
                float py = cy + fy;

                float dx = px - x;
                float dy = py - y;
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist < best) best = dist;
            }
        }

        float mapped = best / 1.41421356f;
        return Sat(1.0f - mapped);
    }

    float SimpleNoise::FBMNoise(float x, float y)
    {
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float sum = 0.0f;
        float max = 0.0f;

        for (int i = 0; i < m_settings.octaves; ++i)
        {
            float nx = x * frequency;
            float ny = y * frequency;
            float val = PerlinNoise(nx, ny);
            sum += val * amplitude;
            max += amplitude;
            amplitude *= m_settings.persistence;
            frequency *= m_settings.lacunarity;
        }

        if (max > 0.0f) sum /= max;
        return sum;
    }

    float SimpleNoise::Fade(float t)
    {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    float SimpleNoise::Lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    float SimpleNoise::Grad(int hash, float x, float y)
    {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v) * 0.5f;
    }

    void SimpleNoise::NormalizeMap(std::vector<float>& map)
    {
        if (map.empty()) return;
        float minv = std::numeric_limits<float>::infinity();
        float maxv = -std::numeric_limits<float>::infinity();
        for (float v : map) { if (v < minv) minv = v; if (v > maxv) maxv = v; }
        if (maxv - minv < 1e-8f)
        {
            for (auto &v : map) v = 0.5f;
            return;
        }
        float invRange = 1.0f / (maxv - minv);
        for (auto &v : map) v = (v - minv) * invRange;
    }
}

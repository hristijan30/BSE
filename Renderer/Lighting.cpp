#include "Lighting.h"
#include <sstream>
#include <algorithm>

namespace BSE
{
    std::vector<LightData> Lighting::s_lights;
    glm::vec3 Lighting::s_ambientColor = glm::vec3(0.03f);
    float Lighting::s_ambientIntensity = 1.0f;
    Lighting::Mode Lighting::s_mode = Lighting::Mode::Lit;

    int Lighting::s_maxLights = Lighting::MaxLights;

    static bool s_frameStarted = false;

    void Lighting::SetMaxLights(int maxLights)
    {
        int clamped = std::max(1, std::min(maxLights, MaxLights));
        s_maxLights = clamped;

        if ((int)s_lights.size() > s_maxLights)
        {
            s_lights.resize(s_maxLights);
        }
    }

    void Lighting::Clear()
    {
        s_lights.clear();
        s_frameStarted = true;
    }

    void Lighting::AddLight(const LightData& light)
    {
        if (!s_frameStarted)
        {
            s_lights.clear();
            s_frameStarted = true;
        }

        if ((int)s_lights.size() >= s_maxLights)
            return;

        s_lights.push_back(light);
    }

    void Lighting::SetAmbient(const glm::vec3& color, float intensity)
    {
        s_ambientColor = color;
        s_ambientIntensity = intensity;
    }

    void Lighting::SetMode(Mode m)
    {
        s_mode = m;
    }

    Lighting::Mode Lighting::GetMode()
    {
        return s_mode;
    }

    static inline std::string IndexName(const char* base, int idx)
    {
        std::ostringstream ss;
        ss << base << "[" << idx << "]";
        return ss.str();
    }

    void Lighting::Apply(GLuint shaderProgram)
    {
        if (shaderProgram == 0) return;

        s_frameStarted = false;

        GLint loc = glGetUniformLocation(shaderProgram, "uAmbientColor");
        if (loc >= 0) glUniform3fv(loc, 1, &s_ambientColor[0]);
        loc = glGetUniformLocation(shaderProgram, "uAmbientIntensity");
        if (loc >= 0) glUniform1f(loc, s_ambientIntensity);

        GLint locMode = glGetUniformLocation(shaderProgram, "uLightingMode");
        if (locMode >= 0) glUniform1i(locMode, (int)s_mode);

        int lightCountToSend = std::min<int>((int)s_lights.size(), s_maxLights);
        GLint locCount = glGetUniformLocation(shaderProgram, "uLightCount");
        if (locCount >= 0) glUniform1i(locCount, lightCountToSend);

        for (int i = 0; i < lightCountToSend; ++i)
        {
            const LightData& L = s_lights[i];

            std::string nameType = IndexName("uLightType", i);
            GLint l = glGetUniformLocation(shaderProgram, nameType.c_str());
            if (l >= 0) glUniform1i(l, (int)L.type);

            std::string namePos = IndexName("uLightPos", i);
            l = glGetUniformLocation(shaderProgram, namePos.c_str());
            if (l >= 0) glUniform3fv(l, 1, &L.position[0]);

            std::string nameDir = IndexName("uLightDir", i);
            l = glGetUniformLocation(shaderProgram, nameDir.c_str());
            if (l >= 0) glUniform3fv(l, 1, &L.direction[0]);

            std::string nameColor = IndexName("uLightColor", i);
            l = glGetUniformLocation(shaderProgram, nameColor.c_str());
            if (l >= 0) glUniform3fv(l, 1, &L.color[0]);

            std::string nameIntensity = IndexName("uLightIntensity", i);
            l = glGetUniformLocation(shaderProgram, nameIntensity.c_str());
            if (l >= 0) glUniform1f(l, L.intensity);

            std::string nameRadius = IndexName("uLightRadius", i);
            l = glGetUniformLocation(shaderProgram, nameRadius.c_str());
            if (l >= 0) glUniform1f(l, L.radius);

            std::string nameInner = IndexName("uLightInnerCone", i);
            l = glGetUniformLocation(shaderProgram, nameInner.c_str());
            if (l >= 0) glUniform1f(l, L.innerCone);

            std::string nameOuter = IndexName("uLightOuterCone", i);
            l = glGetUniformLocation(shaderProgram, nameOuter.c_str());
            if (l >= 0) glUniform1f(l, L.outerCone);

            std::string nameArea = IndexName("uLightAreaSize", i);
            l = glGetUniformLocation(shaderProgram, nameArea.c_str());
            if (l >= 0) glUniform2fv(l, 1, &L.areaSize[0]);
        }
    }

    bool Lighting::ShaderUsesLighting(GLuint shaderProgram)
    {
        if (shaderProgram == 0) return false;
        if (glGetUniformLocation(shaderProgram, "uLightCount") >= 0) return true;
        if (glGetUniformLocation(shaderProgram, "uAmbientColor") >= 0) return true;
        return false;
    }
}

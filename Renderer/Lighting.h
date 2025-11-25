#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include "OpenGL.h"

namespace BSE
{
    enum class LightType : int
    {
        Directional = 0,
        Point       = 1,
        Spot        = 2,
        Area        = 3
    };

    struct LightData
    {
        LightType type = LightType::Directional;
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float radius = 1.0f;
        float innerCone = 0.0f;
        float outerCone = 0.0f;
        glm::vec2 areaSize = glm::vec2(1.0f);
    };

    class DLL_EXPORT Lighting
    {
    public:
        enum class Mode : int
        {
            Unlit = 0,
            Lit = 1
        };

        static void SetMode(Mode m);
        static Mode GetMode();

        static const int MaxLights = 16;

        static void Clear();
        static void AddLight(const LightData& light);

        static void SetAmbient(const glm::vec3& color, float intensity);

        static void Apply(GLuint shaderProgram);
        static bool ShaderUsesLighting(GLuint shaderProgram);

    private:
        static std::vector<LightData> s_lights;
        static glm::vec3 s_ambientColor;
        static float s_ambientIntensity;
        static Mode s_mode;
    };
}

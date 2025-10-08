#pragma once

#include "OpenGL.h"
#include <vector>

namespace BSE
{
    class ColorRamp
    {
    public:
        ColorRamp() = default;

        void Initialize(const std::vector<glm::vec3>& colors)
        {
            m_colors = colors;
        }

        glm::vec3 Sample(float t) const
        {
            if (m_colors.empty())
                return glm::vec3(1.0f, 1.0f, 1.0f);

            t = glm::clamp(t, 0.0f, 1.0f);

            float scaledPos = t * (m_colors.size() - 1);
            int idx0 = static_cast<int>(floor(scaledPos));
            int idx1 = std::min(idx0 + 1, static_cast<int>(m_colors.size() - 1));
            float localT = scaledPos - idx0;

            return glm::mix(m_colors[idx0], m_colors[idx1], localT);
        }

        const std::vector<glm::vec3>& GetColors() const { return m_colors; }

    private:
        std::vector<glm::vec3> m_colors;
    };
}

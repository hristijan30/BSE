#pragma once

#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace BSE::GL
{
    inline void SetDefaultState()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }

    inline void ClearBuffers()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

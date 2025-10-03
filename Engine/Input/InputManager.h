#pragma once

#include "../Define.h"
#include "../StandardInclude.h"
#include "../SDL_Include.h"

#include "Key.h"

#include <glm/glm.hpp>

namespace BSE
{
    class DLL_EXPORT InputManager
    {
    public:
        InputManager();
        ~InputManager() = default;

        void Update();

        bool IsKeyDown(KeyCode key) const;
        bool IsKeyPressed(KeyCode key) const;
        bool IsKeyReleased(KeyCode key) const;

        bool IsMouseButtonDown(int button) const;
        bool IsMouseButtonPressed(int button) const;
        bool IsMouseButtonReleased(int button) const;

        glm::ivec2 GetMousePosition() const { return m_mousePos; }
        glm::ivec2 GetMouseDelta() const    { return m_mouseDelta; }

    private:
        std::vector<bool> m_currentKeys;
        std::vector<bool> m_previousKeys;

        Uint32 m_currentMouseButtons = 0;
        Uint32 m_previousMouseButtons = 0;

        glm::ivec2 m_mousePos{0, 0};
        glm::ivec2 m_lastMousePos{0, 0};
        glm::ivec2 m_mouseDelta{0, 0};

        static Uint32 ButtonToMask(int button);
    };
}

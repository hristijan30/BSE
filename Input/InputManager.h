#pragma once

#include "../Engine/Define.h"
#include "../Engine/StandardInclude.h"
#include "../Engine/SDL_Include.h"

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

        int GetGamepadCount() const;
        bool IsGamepadButtonDown(int padIndex, int button) const;
        bool IsGamepadButtonPressed(int padIndex, int button) const;
        bool IsGamepadButtonReleased(int padIndex, int button) const;
        float GetGamepadAxis(int padIndex, int axis) const;

    private:
        std::vector<bool> m_currentKeys;
        std::vector<bool> m_previousKeys;

        Uint32 m_currentMouseButtons = 0;
        Uint32 m_previousMouseButtons = 0;

        glm::ivec2 m_mousePos{0, 0};
        glm::ivec2 m_lastMousePos{0, 0};
        glm::ivec2 m_mouseDelta{0, 0};

        static Uint32 ButtonToMask(int button);

        void RefreshGamepads();

        struct GamepadState
        {
            SDL_Joystick* joystick = nullptr;
            SDL_JoystickID instanceId = 0;
            std::vector<bool> currentButtons;
            std::vector<bool> previousButtons;
            std::vector<int16_t> axesRaw;
            std::vector<float> axes;
        };

        std::vector<GamepadState> m_gamepads;
    };
}

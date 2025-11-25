#include "InputManager.h"
#include <algorithm>
#include <iostream>

namespace BSE
{
    Uint32 InputManager::ButtonToMask(int button)
    {
    #if defined(SDL_BUTTON_LMASK)
        switch (button)
        {
            case SDL_BUTTON_LEFT:   return SDL_BUTTON_LMASK;
            case SDL_BUTTON_RIGHT:  return SDL_BUTTON_RMASK;
            case SDL_BUTTON_MIDDLE: return SDL_BUTTON_MMASK;
            case SDL_BUTTON_X1:     return SDL_BUTTON_X1MASK;
            case SDL_BUTTON_X2:     return SDL_BUTTON_X2MASK;
            default:                return (1u << (button - 1));
        }
    #else
        // Fallback: use bit shift like SDL_BUTTON macro used to produce.
        return (1u << (button - 1));
    #endif
    }

    InputManager::InputManager()
    {
        int numKeys = 0;
        const bool* state = SDL_GetKeyboardState(&numKeys);

        m_currentKeys.resize(numKeys);
        m_previousKeys.resize(numKeys);

        if (state && numKeys > 0)
        {
            std::copy(state, state + numKeys, m_currentKeys.begin());
            std::copy(state, state + numKeys, m_previousKeys.begin());
        }
        else
        {
            m_currentKeys.clear();
            m_previousKeys.clear();
        }

        float fx = 0.0f, fy = 0.0f;
        m_currentMouseButtons = SDL_GetMouseState(&fx, &fy);
        m_mousePos = { static_cast<int>(fx), static_cast<int>(fy) };
        m_lastMousePos = m_mousePos;
        m_previousMouseButtons = m_currentMouseButtons;

        RefreshGamepads();
    }

    void InputManager::Update()
    {
        m_previousKeys = m_currentKeys;
        m_previousMouseButtons = m_currentMouseButtons;
        m_lastMousePos = m_mousePos;

        SDL_PumpEvents();

        int numKeys = 0;
        const bool* state = SDL_GetKeyboardState(&numKeys);
        if (state && numKeys > 0)
        {
            if (static_cast<int>(m_currentKeys.size()) != numKeys)
            {
                m_currentKeys.resize(numKeys);
                m_previousKeys.resize(numKeys);
            }
            m_currentKeys.assign(state, state + numKeys);
        }

        float fx = 0.0f, fy = 0.0f;
        m_currentMouseButtons = SDL_GetMouseState(&fx, &fy);
        m_mousePos = { static_cast<int>(fx), static_cast<int>(fy) };
        m_mouseDelta = m_mousePos - m_lastMousePos;

        RefreshGamepads();

        for (auto &g : m_gamepads)
        {
            if (!g.joystick) continue;

            g.previousButtons = g.currentButtons;

            int btnCount = SDL_GetNumJoystickButtons(g.joystick);
            if (btnCount > 0)
            {
                if ((int)g.currentButtons.size() != btnCount)
                {
                    g.currentButtons.assign(btnCount, false);
                    g.previousButtons.assign(btnCount, false);
                }

                for (int b = 0; b < btnCount; ++b)
                {
                    g.currentButtons[b] = (SDL_GetJoystickButton(g.joystick, b) != 0);
                }
            }

            int axisCount = SDL_GetNumJoystickAxes(g.joystick);
            if (axisCount > 0)
            {
                if ((int)g.axesRaw.size() != axisCount)
                {
                    g.axesRaw.assign(axisCount, 0);
                    g.axes.assign(axisCount, 0.0f);
                }

                for (int a = 0; a < axisCount; ++a)
                {
                    int16_t raw = SDL_GetJoystickAxis(g.joystick, a);
                    g.axesRaw[a] = raw;
                    // Normalize to -1..1 (SDL_JOYSTICK_AXIS_MAX == 32767)
                    g.axes[a] = (raw >= 0) ? (raw / 32767.0f) : (raw / 32768.0f);
                }
            }
        }
    }

    bool InputManager::IsKeyDown(KeyCode key) const
    {
        int sc = static_cast<int>(key);
        if (sc < 0) return false;
        if (sc >= static_cast<int>(m_currentKeys.size())) return false;
        return m_currentKeys[sc];
    }

    bool InputManager::IsKeyPressed(KeyCode key) const
    {
        int sc = static_cast<int>(key);
        if (sc < 0) return false;
        if (sc >= static_cast<int>(m_currentKeys.size())) return false;
        return (m_currentKeys[sc] && !m_previousKeys[sc]);
    }

    bool InputManager::IsKeyReleased(KeyCode key) const
    {
        int sc = static_cast<int>(key);
        if (sc < 0) return false;
        if (sc >= static_cast<int>(m_currentKeys.size())) return false;
        return (!m_currentKeys[sc] && m_previousKeys[sc]);
    }

    bool InputManager::IsMouseButtonDown(int button) const
    {
        Uint32 mask = ButtonToMask(button);
        return (m_currentMouseButtons & mask) != 0;
    }

    bool InputManager::IsMouseButtonPressed(int button) const
    {
        Uint32 mask = ButtonToMask(button);
        return ((m_currentMouseButtons & mask) != 0) && ((m_previousMouseButtons & mask) == 0);
    }

    bool InputManager::IsMouseButtonReleased(int button) const
    {
        Uint32 mask = ButtonToMask(button);
        return ((m_currentMouseButtons & mask) == 0) && ((m_previousMouseButtons & mask) != 0);
    }

    int InputManager::GetGamepadCount() const
    {
        return static_cast<int>(m_gamepads.size());
    }

    bool InputManager::IsGamepadButtonDown(int padIndex, int button) const
    {
        if (padIndex < 0 || padIndex >= (int)m_gamepads.size()) return false;
        const auto &g = m_gamepads[padIndex];
        if (!g.joystick) return false;
        if (button < 0 || button >= (int)g.currentButtons.size()) return false;
        return g.currentButtons[button];
    }

    bool InputManager::IsGamepadButtonPressed(int padIndex, int button) const
    {
        if (padIndex < 0 || padIndex >= (int)m_gamepads.size()) return false;
        const auto &g = m_gamepads[padIndex];
        if (!g.joystick) return false;
        if (button < 0 || button >= (int)g.currentButtons.size()) return false;
        return g.currentButtons[button] && !g.previousButtons[button];
    }

    bool InputManager::IsGamepadButtonReleased(int padIndex, int button) const
    {
        if (padIndex < 0 || padIndex >= (int)m_gamepads.size()) return false;
        const auto &g = m_gamepads[padIndex];
        if (!g.joystick) return false;
        if (button < 0 || button >= (int)g.currentButtons.size()) return false;
        return !g.currentButtons[button] && g.previousButtons[button];
    }

    float InputManager::GetGamepadAxis(int padIndex, int axis) const
    {
        if (padIndex < 0 || padIndex >= (int)m_gamepads.size()) return 0.0f;
        const auto &g = m_gamepads[padIndex];
        if (!g.joystick) return 0.0f;
        if (axis < 0 || axis >= (int)g.axes.size()) return 0.0f;
        return g.axes[axis];
    }

    void InputManager::RefreshGamepads()
    {
        int count = 0;
        SDL_JoystickID *ids = SDL_GetJoysticks(&count);
        if (!ids)
        {
            return;
        }

        std::vector<SDL_JoystickID> currentIds;
        currentIds.assign(ids, ids + count);
        SDL_free(ids);

        for (int i = (int)m_gamepads.size() - 1; i >= 0; --i)
        {
            auto &g = m_gamepads[i];
            bool found = false;
            for (auto id : currentIds) if (id == g.instanceId) { found = true; break; }
            if (!found)
            {
                if (g.joystick) SDL_CloseJoystick(g.joystick);
                m_gamepads.erase(m_gamepads.begin() + i);
            }
        }

        for (auto id : currentIds)
        {
            bool already = false;
            for (const auto &g : m_gamepads) if (g.instanceId == id) { already = true; break; }
            if (already) continue;

            SDL_Joystick *joy = SDL_OpenJoystick(id);
            if (!joy) continue;

            GamepadState gs;
            gs.joystick = joy;
            gs.instanceId = id;

            int btnCount = SDL_GetNumJoystickButtons(joy);
            if (btnCount > 0)
            {
                gs.currentButtons.assign(btnCount, false);
                gs.previousButtons.assign(btnCount, false);
            }

            int axisCount = SDL_GetNumJoystickAxes(joy);
            if (axisCount > 0)
            {
                gs.axesRaw.assign(axisCount, 0);
                gs.axes.assign(axisCount, 0.0f);
            }

            m_gamepads.push_back(gs);
        }
    }
}

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
}

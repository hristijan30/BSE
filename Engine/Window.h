#pragma once

#include "Define.h"
#include "StandardInclude.h"

#include "Renderer/OpenGL.h"
#include "SDL_Include.h"

namespace BSE
{
    class DLL_EXPORT Window
    {
    public:
        Window(const char* title, int width, int height, bool resizable = true, bool fullscreen = false);
        ~Window();

        void Create();
        void Destroy();
        void SwapBuffers();
        
        bool IsOpen() const { return m_window != nullptr; }
        SDL_Window* GetSDLWindow() const { return m_window; }

    private:
        const char* m_title;
        int m_width;
        int m_height;
        bool m_resizable;
        bool m_fullscreen;

        SDL_Window* m_window;
        SDL_GLContext m_glContext;
    };
}

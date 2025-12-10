#include "Window.h"

namespace BSE
{
    Window::Window(const char* title, int width, int height, bool resizable, bool fullscreen, bool vsync)
        : m_title(title), m_width(width), m_height(height), m_resizable(resizable), m_fullscreen(fullscreen), m_vsync(vsync),
          m_window(nullptr), m_glContext(nullptr)
    {
    }

    Window::~Window()
    {
        Destroy();
    }

    void Window::Create()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        Uint32 flags = SDL_WINDOW_OPENGL;
        if (m_resizable) flags |= SDL_WINDOW_RESIZABLE;
        if (m_fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

        m_window = SDL_CreateWindow(m_title, m_width, m_height, flags);
        if (!m_window)
        {
            throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
        }

        m_glContext = SDL_GL_CreateContext(m_window);
        if (!m_glContext)
        {
            throw std::runtime_error("SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
        }

        if (SDL_GL_MakeCurrent(m_window, m_glContext) < 0)
        {
            throw std::runtime_error("SDL_GL_MakeCurrent failed: " + std::string(SDL_GetError()));
        }

        if (m_vsync == true)
        {
            if (SDL_GL_SetSwapInterval(1) < 0)
            {
                throw std::runtime_error("SDL_GL_SetSwapInterval failed: " + std::string(SDL_GetError()));
            }
        }

        GL::SetDefaultState();

        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK)
        {
            std::ostringstream oss;
            oss << "glewInit failed: " << reinterpret_cast<const char*>(glewGetErrorString(glewStatus));
            throw std::runtime_error(oss.str());
        }

        glGetError();

        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "Renderer:       " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Vendor:         " << glGetString(GL_VENDOR) << std::endl;
    }

    void Window::Destroy()
    {
        if (m_glContext)
        {
            SDL_GL_DestroyContext(m_glContext);
            m_glContext = nullptr;
        }

        if (m_window)
        {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }

        SDL_Quit();
    }

    void Window::SwapBuffers()
    {
        SDL_GL_SwapWindow(m_window);
    }
}

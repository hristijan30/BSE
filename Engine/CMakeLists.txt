
set(SOURCES
    "Renderer/OpenGL.h"
    "Renderer/Shader.cpp"
    "Renderer/Shader.h"
    "Renderer/Camera.cpp"
    "Renderer/Camera.h"
    "Sound/Sound.cpp"
    "Sound/Sound.h"
    "Sound/SoundManager.cpp"
    "Sound/SoundManager.h"
    "Threading/ThreadingSystem.cpp"
    "Threading/ThreadingSystem.h"
    "Define.h"
    "SDL_Include.h"
    "Engine.cpp"
    "Engine.h"
    "Logger.cpp"
    "Logger.h"
    "Time.cpp"
    "Time.h"
    "Window.cpp"
    "Window.h"
)

add_library(BSE_Core SHARED "${SOURCES}")

target_link_libraries(BSE_Core PUBLIC SDL3::SDL3 OpenGL::GL GLEW::GLEW TBB::tbb)

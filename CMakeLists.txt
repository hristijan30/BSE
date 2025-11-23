cmake_minimum_required(VERSION 3.19)
project(BSE VERSION 0.1.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib")

cmake_policy(SET CMP0167 NEW)

find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Boost CONFIG REQUIRED)

add_subdirectory("ThirdParty/ReactPhysics3d")

if(NOT SDL3_FOUND)
    message(WARNING "SDL3 not found — using ThirdParty/SDL3 and raw .lib")
    add_subdirectory("ThirdParty/SDL3")
    set(USE_RAW_SDL3_LIB TRUE)
endif()

set(CORE_SOURCE
    "Engine/Define.h"
    "Engine/SDL_Include.h"
    "Engine/StandardInclude.h"
    "Engine/Engine.cpp"
    "Engine/Engine.h"
    "Engine/Logger.cpp"
    "Engine/Logger.h"
    "Engine/Time.cpp"
    "Engine/Time.h"
    "Engine/Window.cpp"
    "Engine/Window.h"
)

set(EVENT_SOURCE
    "Events/WindowEvent.cpp"
    "Events/WindowEvent.h"
)

set(INPUT_SOURCE
    "Input/Key.h"
    "Input/InputManager.cpp"
    "Input/InputManager.h"
)

set(NETWORKING_SOURCE
    "Networking/Cerialization.cpp"
    "Networking/Cerialization.h"
    "Networking/Client.cpp"
    "Networking/Client.h"
    "Networking/Connection.cpp"
    "Networking/Connection.h"
    "Networking/ServerMain.cpp"
    "Networking/ServerMain.h"
)

set(NODE_SOURCE
    "NodeGraph/Components.h"
    "NodeGraph/Node.h"
)

set(PHYSICS_SOURCE
    "Physics/React.h"
    "Physics/PhysicsMaterial.cpp"
    "Physics/PhysicsMaterial.h"
    "Physics/PhysicsBody.cpp"
    "Physics/PhysicsBody.h"
)

set(RENDERER_SOURCE
    "Renderer/OpenGL.h"
    "Renderer/Buffer.cpp"
    "Renderer/Buffer.h"
    "Renderer/Lighting.cpp"
    "Renderer/Lighting.h"
    "Renderer/Material.cpp"
    "Renderer/Material.h"
    "Renderer/Model.cpp"
    "Renderer/Model.h"
    "Renderer/Shader.cpp"
    "Renderer/Shader.h"
    "Renderer/Texture2D.cpp"
    "Renderer/Texture2D.h"
)

set(SOUND_SOURCE
    "Sound/OpenAL.h"
    "Sound/Sound.cpp"
    "Sound/Sound.h"
    "Sound/SoundManager.cpp"
    "Sound/SoundManager.h"
)

set(THREADING_SOURCE
    "Threading/ThreadingSystem.cpp"
    "Threading/ThreadingSystem.h"
)

if(WIN32)
    add_library(BSE_Engine SHARED
        "${CORE_SOURCE}"
        "${EVENT_SOURCE}"
        "${INPUT_SOURCE}"
        "${NETWORKING_SOURCE}"
        "${NODE_SOURCE}"
        "${PHYSICS_SOURCE}"
        "${RENDERER_SOURCE}"
        "${SOUND_SOURCE}"
        "${THREADING_SOURCE}"
    )
else()
    add_library(BSE_Engine STATIC
        "${CORE_SOURCE}"
        "${EVENT_SOURCE}"
        "${INPUT_SOURCE}"
        "${NETWORKING_SOURCE}"
        "${NODE_SOURCE}"
        "${PHYSICS_SOURCE}"
        "${RENDERER_SOURCE}"
        "${SOUND_SOURCE}"
        "${THREADING_SOURCE}"
    )
endif()

if(USE_RAW_SDL3_LIB)
    target_include_directories(BSE_Engine PUBLIC
        "${CMAKE_SOURCE_DIR}/ThirdParty/SDL3/include"
    )
endif()

if(WIN32)
    if(USE_RAW_SDL3_LIB)
        target_link_libraries(BSE_Engine PUBLIC
            "SDL3.lib"
            OpenGL::GL
            GLEW::GLEW
            TBB::tbb
            OpenAL::OpenAL
            reactphysics3d
            Boost::boost
            ws2_32
            mswsock
        )
    else()
        target_link_libraries(BSE_Engine PUBLIC
            SDL3::SDL3
            OpenGL::GL
            GLEW::GLEW
            TBB::tbb
            OpenAL::OpenAL
            reactphysics3d
            Boost::boost
            ws2_32
            mswsock
        )
    endif()
else()
    if(USE_RAW_SDL3_LIB)
        target_link_libraries(BSE_Engine PUBLIC
            "libSDL3.a"
            OpenGL::GL
            GLEW::GLEW
            TBB::tbb
            OpenAL::OpenAL
            reactphysics3d
            Boost::boost
        )
    else()
        target_link_libraries(BSE_Engine PUBLIC
            SDL3::SDL3
            OpenGL::GL
            GLEW::GLEW
            TBB::tbb
            OpenAL::OpenAL
            reactphysics3d
            Boost::boost
        )
    endif()
endif()

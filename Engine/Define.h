#pragma once

#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT // Blank for linux
#endif

constexpr float PI_F = 3.1415927f;
constexpr double PI_D = 3.141592653589793;

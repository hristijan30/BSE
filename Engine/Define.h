#pragma once

#ifndef DLL_EXPORT
#  if defined(_WIN32) || defined(_WIN64)
#     define DLL_EXPORT __declspec(dllexport)
#  else
#    define DLL_EXPORT
#  endif
#endif

constexpr float PI_F = 3.1415927f;
constexpr double PI_D = 3.141592653589793;

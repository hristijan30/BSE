cmake_minimum_required(VERSION 3.16)
project(BSE VERSION 0.0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Lib")

find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(TBB REQUIRED)
#find_package(Vulkan REQUIRED) ---> Will use in future

add_executable(TEST_WINDOW WIN32 "Test/WindowMain.cpp")
target_link_libraries(TEST_WINDOW PRIVATE BSE_Core)

add_executable(STATISTICS_TEST "Test/Statistics.cpp")
target_link_libraries(STATISTICS_TEST PRIVATE BSE_Core)

add_subdirectory("Engine")

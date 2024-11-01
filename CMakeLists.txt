cmake_minimum_required(VERSION 3.29)
project(firstapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3.6 REQUIRED)

add_custom_target(
        shaders
        COMMAND python3 ${CMAKE_SOURCE_DIR}/compile_shaders.py
)

add_executable(foobar
        Window.cpp
        Window.h
)

target_sources(foobar PRIVATE main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
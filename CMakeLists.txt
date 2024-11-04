cmake_minimum_required(VERSION 3.29)
project(firstapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3.6 REQUIRED)

add_executable(foobar
        Window.cpp
        Window.h
        device.h
        device.cpp
        Pipeline.cpp
        Pipeline.h
        files.cpp
        pipeline/VertexStageParamsBuilder.h
        pipeline/VertexStageParamsBuilder.cpp
        pipeline/PipelineBuilder.h
        pipeline/PipelineBuilder.cpp
        pipeline/FragmentStageParamsBuilder.cpp
        pipeline/FragmentStageParamsBuilder.h
        pipeline/builders.h
)

add_custom_command(
        TARGET foobar PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/buildscripts/compile-shaders.py
)

target_sources(foobar PRIVATE main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
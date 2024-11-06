cmake_minimum_required(VERSION 3.29)
project(firstapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

# Verify the path to glfw3.lib
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw")
    message(FATAL_ERROR "Missing ${CMAKE_SOURCE_DIR}/libs/glfw")
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/libs/glfw")
include_directories("${CMAKE_SOURCE_DIR}/libs/glfw/include")

add_executable(foobar
        src/Window.cpp
        src/Window.h
        src/device.h
        src/device.cpp
        src/Pipeline.cpp
        src/Pipeline.h
        src/files.cpp
        src/pipeline/VertexStageParamsBuilder.h
        src/pipeline/VertexStageParamsBuilder.cpp
        src/pipeline/PipelineBuilder.h
        src/pipeline/PipelineBuilder.cpp
        src/pipeline/FragmentStageParamsBuilder.cpp
        src/pipeline/FragmentStageParamsBuilder.h
        src/pipeline/builders.h
        src/shaders.h
        src/shaders.cpp
        src/files.h
        src/pipeline/PipelineInputAssemblyStateBuilder.cpp
        src/pipeline/PipelineInputAssemblyStateBuilder.h
        src/pipeline/PipelineViewportStateBuilder.cpp
        src/pipeline/PipelineViewportStateBuilder.h
        src/pipeline/PipelineRasterizationStateBuilder.cpp
        src/pipeline/PipelineRasterizationStateBuilder.h
        src/pipeline/PipelineMultisampleStateBuilder.cpp
        src/pipeline/PipelineMultisampleStateBuilder.h
        src/pipeline/PipelineColorBlendStateBuilder.cpp
        src/pipeline/PipelineColorBlendStateBuilder.h
        src/pipeline/PipelineDepthStencilStateBuilder.cpp
        src/pipeline/PipelineDepthStencilStateBuilder.h
)

add_custom_command(
        TARGET foobar PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/buildscripts/compile-shaders.py
)

target_sources(foobar PRIVATE src/main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
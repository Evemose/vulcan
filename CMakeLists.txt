cmake_minimum_required(VERSION 3.29)
project(firstapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

# Unix requires the glfw library to be installed, due to some unknown issue with build from source
# that causes window to not open
if (UNIX)
    find_package(glfw3 REQUIRED)
endif()

# Windows requires the glfw library to be built from source
if (WIN32)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw")
        message(FATAL_ERROR "Missing ${CMAKE_SOURCE_DIR}/libs/glfw")
    endif()

    add_subdirectory("${CMAKE_SOURCE_DIR}/libs/glfw")
    include_directories("${CMAKE_SOURCE_DIR}/libs/glfw/include")
endif ()

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
        src/pipeline/PipelineVertexInputStateBuilder.cpp
        src/pipeline/PipelineVertexInputStateBuilder.h
        src/pipeline/PipelineLayoutBuilder.cpp
        src/pipeline/PipelineLayoutBuilder.h
        src/swap_chain.h
        src/swap_chain.cpp
)

add_custom_command(
        TARGET foobar PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/buildscripts/compile-shaders.py
)

target_sources(foobar PRIVATE src/main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
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
        src/main.cpp
        src/IRenderer.h
        src/Mesh.h
        src/RenderObject.h
        src/vulkan/VulkanRenderer.cpp
        src/vulkan/VulkanRenderer.h
        src/vulkan/VulkanRendererConstructor.cpp
        src/vulkan/VulkanRendererConstructor.h
        src/vulkan/structs.h
        src/vulkan/utils.h
        src/io/files.h
        src/vulkan/MeshBuffers.cpp
        src/vulkan/MeshBuffers.h
        src/vulkan/utils.cpp
)

add_custom_target(CompileShaders ALL
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/buildscripts/compile-shaders.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


add_dependencies(foobar CompileShaders)

target_sources(foobar PRIVATE src/main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
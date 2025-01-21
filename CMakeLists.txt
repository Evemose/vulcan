cmake_minimum_required(VERSION 3.29)
project(firstapp)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

if (WIN32)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/glfw")
        message(FATAL_ERROR "Missing ${CMAKE_SOURCE_DIR}/libs/glfw")
    endif()
    add_subdirectory("${CMAKE_SOURCE_DIR}/libs/glfw")
    include_directories("${CMAKE_SOURCE_DIR}/libs/glfw/include")
elseif (UNIX)
    find_package(glfw3 REQUIRED)
endif()

add_executable(foobar
        src/main.cpp
        src/VulkanRenderer.cpp
        src/VulkanRenderer.h
        src/swapchain_choices.h
        src/swapchain_choices.cpp
        src/utils.h
)

add_custom_target(CompileShaders ALL
        COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_SOURCE_DIR}/buildscripts/compile-shaders.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compiling shaders..."
)

add_dependencies(foobar CompileShaders)

target_sources(foobar PRIVATE src/main.cpp)

target_link_libraries(foobar PRIVATE Vulkan::Vulkan)
target_link_libraries(foobar PRIVATE glfw)
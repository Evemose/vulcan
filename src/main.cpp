#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>

#include "VulkanRenderer.h"

GLFWwindow* createWindow(const std::string &name = "Test Window", const int width = 800, const int height = 600) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}

int main() {
    GLFWwindow* window = createWindow();
    auto renderer = VulkanRenderer::create(window);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderer.drawFrame({
            {{0.5f, 0.5f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}},
            {{-0.5f, -0.5f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}}
        });
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
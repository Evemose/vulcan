#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "vulkan/VulkanRendererConstructor.h"

using WindowPtr = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>;

WindowPtr createWindow(const std::string& title, int width, int height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* rawWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!rawWindow) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    return { rawWindow, glfwDestroyWindow };
}

int main() {
    auto window = createWindow("Hello World!", 800, 600);
    auto renderer = enjine::VulkanRendererConstructor().create(window.get());

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

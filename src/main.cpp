#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "vulkan/VulkanRendererConstructor.h"

using WindowPtr = std::unique_ptr<GLFWwindow, void(*)(GLFWwindow *)>;

WindowPtr createWindow(const std::string &title, int width, int height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *rawWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!rawWindow) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    return {rawWindow, glfwDestroyWindow};
}

int main() {
    auto window = createWindow("Hello World!", 800, 600);
    auto renderer = enjine::VulkanRendererConstructor().create(window.get());

    std::vector<enjine::RenderObject> renderObjects;

    std::vector<enjine::Vertex> vertices;
    vertices.push_back(enjine::Vertex{{-0.6f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back(enjine::Vertex{{-0.1f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back(enjine::Vertex{{-0.1f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    renderObjects.push_back(enjine::RenderObject{
        enjine::Mesh {
            vertices,
            indices
        },
        glm::mat4{1.0f},
    });

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
        renderer->render(renderObjects);
    }

    glfwTerminate();
    return 0;
}

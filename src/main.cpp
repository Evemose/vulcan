#include <stdexcept>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

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

glm::vec3 generateRandomColor() {
    return {
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX
    };
}

int main() {
    auto window = createWindow("Hello World!", 800, 600);
    auto renderer = enjine::VulkanRendererConstructor().create(window.get());

    std::vector<enjine::RenderObject> renderObjects;

    std::vector<enjine::Vertex> vertices;
    vertices.push_back(enjine::Vertex{{-0.6f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back(enjine::Vertex{{-0.1f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back(enjine::Vertex{{-0.1f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    vertices.push_back(enjine::Vertex{{-0.6f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}});

    std::vector<uint32_t> indices = {0, 2, 1, 0, 3, 2};

    renderObjects.push_back(enjine::RenderObject{
        enjine::Mesh{
            vertices,
            indices
        },
        {
            rotate(glm::mat4{1.0f}, glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f}),
        },
    });

    std::vector<enjine::Vertex> circleVertices;
    constexpr int numSegments = 100;
    for (int i = 0; i < numSegments; ++i) {
        constexpr float radius = 0.5f;
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(numSegments);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        circleVertices.push_back(enjine::Vertex{{x, y, 0.0f}, {1.0f, 1.0f, 0.0f}});
    }

    std::vector<uint32_t> circleIndices;
    for (int i = 0; i < numSegments; ++i) {
        circleIndices.push_back(i);
        circleIndices.push_back((i + 1) % numSegments);
        circleIndices.push_back(numSegments);
    }
    circleVertices.push_back(enjine::Vertex{{0.0f, 0.0f, 0.0f}, generateRandomColor()});

    renderObjects.push_back(enjine::RenderObject{
        enjine::Mesh{
            circleVertices,
            circleIndices
        },
        {
            translate(glm::mat4{1.0f}, glm::vec3{0.5f, 0.7f, 0.0f}) *
            rotate(glm::mat4{1.0f}, glm::radians(45.0f), glm::vec3{0.4f, 1.0f, 1.0f}),
        },
    });

    auto projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(renderer->getExtent().width) / static_cast<float>(renderer->getExtent().height),
        0.1f, 10.0f
    );
    projection[1][1] *= -1;
    auto view = lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
        renderer->render(
            renderObjects, {
                view,
                projection
            }
        );
    }

    glfwTerminate();
    return 0;
}

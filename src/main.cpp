#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <random>

#include "VulkanRenderer.h"

GLFWwindow *createWindow(const std::string &name = "Test Window", const int width = 800, const int height = 600) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}

glm::vec3 randomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution dis(0.0f, 1.0f);

    return {
        dis(gen),
        dis(gen),
        dis(gen)
    };
}

int main() {
    GLFWwindow *window = createWindow();
    auto renderer = VulkanRenderer::create(window);
    auto meshFactory = renderer.getMeshFactory();

    auto firstColor = randomColor();
    auto secondColor = randomColor();
    auto thirdColor = randomColor();
    auto fourthColor = randomColor();

    std::vector<Vertex> vertices1 = {
        {{-0.6f, -0.5f, 0.0f}, firstColor},
        {{-0.1f, -0.5f, 0.0f}, secondColor},
        {{-0.1f, 0.5f, 0.0f}, thirdColor},
        {{-0.6f, 0.3f, 0.0f}, fourthColor},
    };
    std::vector<Vertex> vertices2 = {
        {{0.1f, -0.5f, 0.0f}, firstColor},
        {{0.6f, -0.5f, 0.0f}, secondColor},
        {{0.6f, 0.5f, 0.0f}, thirdColor},
        {{0.1f, 0.9f, 0.0f}, fourthColor},
    };
    std::vector<uint32_t> indices1 = {0, 1, 2, 0, 2, 3};
    std::vector<uint32_t> indices2 = {0, 1, 2, 0, 2, 3};

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

    auto mesh1 = meshFactory.createMesh(vertices1, indices);
    auto mesh2 = meshFactory.createMesh(vertices2, indices);

    std::vector<Mesh> meshes;
    meshes.reserve(2);
    meshes.push_back(std::move(mesh1));
    meshes.push_back(std::move(mesh2));

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderer.drawFrame(meshes);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

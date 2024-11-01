#include "Window.h"

int main() {
    const auto window = Window(800, 600, "Vulkan");
    while (!window.shouldClose()) {
        glfwPollEvents();
    }
}

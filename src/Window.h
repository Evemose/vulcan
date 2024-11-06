#ifndef WINDOW_H
#define WINDOW_H
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace std;

class Window {
private:
    int const width;
    int const height;
    std::string const title;
    GLFWwindow* const window;
    static GLFWwindow* createWindow(int width, int height, const std::string& title);
public:
    Window(int width, int height, std::string title);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    [[nodiscard]] bool shouldClose() const;
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;
};



#endif //WINDOW_H

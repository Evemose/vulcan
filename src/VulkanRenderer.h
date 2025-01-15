#ifndef VULKARENDERER_H
#define VULKARENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class VulkanRenderer {
public:
    static VulkanRenderer create(GLFWwindow* window);
    ~VulkanRenderer();
private:
    const GLFWwindow* window;
    const VkInstance instance;
    VulkanRenderer(const GLFWwindow *window, VkInstance instance);

    static std::vector<const char *> getExtensions();

    static VkApplicationInfo getAppInfo();

    static VkInstance createInstance();
};



#endif //VULKARENDERER_H

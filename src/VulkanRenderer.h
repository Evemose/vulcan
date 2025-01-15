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
    typedef struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } Device;

    const GLFWwindow* window;
    const VkInstance instance;
    const Device device{};

    VulkanRenderer(const GLFWwindow *window, VkInstance instance, Device device);

    static std::vector<const char *> getExtensions();
    static VkApplicationInfo getAppInfo();
    static VkInstance createInstance();

    static VkPhysicalDevice get_physical_device(VkInstance instance);

    static Device getDevice(VkInstance instance);
};



#endif //VULKARENDERER_H

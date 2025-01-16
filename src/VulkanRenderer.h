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

    typedef struct {
        int graphicsFamily = -1;

        [[nodiscard]] bool isValid() const {
            return graphicsFamily >= 0;
        }
    } QueueFamilyIndices;

    const GLFWwindow* window;
    const VkInstance instance;
    const Device device{};
    const QueueFamilyIndices queues;

    VulkanRenderer(const GLFWwindow *window, VkInstance instance, Device device, QueueFamilyIndices queues);

    static std::vector<const char *> getExtensions();
    static VkApplicationInfo getAppInfo();
    static VkInstance createInstance();

    static VkPhysicalDevice getPhysicalDevice(VkInstance instance);

    static VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices queues);

    static std::tuple<Device, QueueFamilyIndices>  getDevice(VkInstance instance);

    static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

    static [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device);
};



#endif //VULKARENDERER_H

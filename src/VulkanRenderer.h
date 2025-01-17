#ifndef VULKARENDERER_H
#define VULKARENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <tuple>
#include <GLFW/glfw3.h>
#include <vector>

class VulkanRenderer {
public:
    static VulkanRenderer create(GLFWwindow *window);

    ~VulkanRenderer();

private:
    typedef struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } Device;

    typedef struct {
        int graphicsFamily = -1;
        int presentFamily = -1;

        [[nodiscard]] bool isValid() const {
            return graphicsFamily >= 0
                   && presentFamily >= 0;
        }
    } QueueFamilyIndices;

    typedef struct {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        [[nodiscard]] bool isValid() const {
            return !formats.empty() && !presentModes.empty();
        }
    } SwapChainDetails;

    const GLFWwindow *window;
    const VkInstance instance;
    const Device device{};
    const QueueFamilyIndices queues;
    const VkSurfaceKHR surface;
    const VkQueue graphicsQueue;
    const VkQueue presentQueue;

    VulkanRenderer(const GLFWwindow *window, VkInstance instance, Device device, QueueFamilyIndices queues,
                   VkSurfaceKHR surface);

    static VkPhysicalDevice getPhysicalDevice(VkInstance instance, VkSurfaceKHR surfaceTuSupport);

    static std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfos(QueueFamilyIndices queues);

    static VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices queues);

    static std::tuple<Device, QueueFamilyIndices, SwapChainDetails> getDeviceAndDetails(VkInstance instance, VkSurfaceKHR surfaceToSupport);

    static SwapChainDetails getSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

    static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surfaceTuSupport);

    static [[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surfaceTuSupport);
};


#endif //VULKARENDERER_H

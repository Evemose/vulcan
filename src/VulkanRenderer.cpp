#include "VulkanRenderer.h"

#include <cstring>
#include <set>

#include "utils.h"
#include <stdexcept>
#include <tuple>

const auto requiredExtensions = std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    return surface;
}

std::vector<const char*> getExtensions() {
    auto extensions = std::vector<const char*>();
    uint32_t glfwExtensionsCount;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    for (auto i = 0u; i < glfwExtensionsCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

VkApplicationInfo getAppInfo() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    return appInfo;
}

VkInstance createInstance() {
    const VkApplicationInfo appInfo = getAppInfo();

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

    return instance;
}

VulkanRenderer VulkanRenderer::create(GLFWwindow* window) {
    auto instance = createInstance();
    auto surface = createSurface(instance, window);
    auto [device, queues, swapChainDetails] = getDeviceAndDetails(instance, surface);
    return {
        window, instance, device, queues, surface
    };
}

VkQueue getGraphicsQueue(const VkDevice device, const int graphicsFamilyIdx) {
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, graphicsFamilyIdx, 0, &graphicsQueue);
    return graphicsQueue;
}

VkQueue getPresentQueue(const VkDevice device, const int presentFamilyIdx) {
    VkQueue presentQueue;
    vkGetDeviceQueue(device, presentFamilyIdx, 0, &presentQueue);
    return presentQueue;
}

VulkanRenderer::VulkanRenderer(const GLFWwindow* window, const VkInstance instance, const Device device,
                               const QueueFamilyIndices queues, const VkSurfaceKHR surface)
    : window(window), instance(instance), device(device), queues(queues), surface(surface),
      graphicsQueue(getGraphicsQueue(device.logicalDevice, queues.graphicsFamily)),
      presentQueue(getPresentQueue(device.logicalDevice, queues.presentFamily)) {
}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(instance, nullptr);
    vkDestroyDevice(device.logicalDevice, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkPhysicalDevice VulkanRenderer::getPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surfaceTuSupport) {
    std::vector<VkPhysicalDevice> devices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan devices found");
    }
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device, surfaceTuSupport)) {
            return device;
        }
    }

    throw std::runtime_error("No suitable Vulkan devices found");
}

std::vector<VkDeviceQueueCreateInfo> VulkanRenderer::getQueueCreateInfos(const QueueFamilyIndices queues) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (std::set uniqueQueueFamilies = {queues.graphicsFamily, queues.presentFamily}; auto idx : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = idx;
        queueCreateInfo.queueCount = 1;
        constexpr float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    return queueCreateInfos;
}

std::vector<VkExtensionProperties> getAvailableExtensions(const VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    auto availableExtensions = std::vector<VkExtensionProperties>(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    return availableExtensions;
}

VkDevice VulkanRenderer::createLogicalDevice(const VkPhysicalDevice physicalDevice, const QueueFamilyIndices queues) {
    const auto queueCreateInfos = getQueueCreateInfos(queues);
    const auto extensions = getAvailableExtensions(physicalDevice);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice logicalDevice;
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

    return logicalDevice;
}

std::tuple<VulkanRenderer::Device, VulkanRenderer::QueueFamilyIndices, VulkanRenderer::SwapChainDetails>
VulkanRenderer::getDeviceAndDetails(
    const VkInstance instance, const VkSurfaceKHR surfaceToSupport
) {
    const auto physicalDevice = getPhysicalDevice(instance, surfaceToSupport);
    const auto queues = getQueueFamilies(physicalDevice, surfaceToSupport);
    const auto swapChainDetails = getSwapChainDetails(physicalDevice, surfaceToSupport);
    const auto logicalDevice = createLogicalDevice(physicalDevice, queues);
    const Device device = {physicalDevice, logicalDevice};
    return {device, queues, swapChainDetails};
}

VulkanRenderer::SwapChainDetails VulkanRenderer::getSwapChainDetails(
    const VkPhysicalDevice device, const VkSurfaceKHR surface
) {
    SwapChainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::getQueueFamilies(
    const VkPhysicalDevice device, const VkSurfaceKHR surfaceTuSupport
) {
    QueueFamilyIndices indices;
    auto queueFamilyCount = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (auto i = 0; i < queueFamilyCount && !indices.isValid(); i++) {
        const auto queueFamily = queueFamilies[i];
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surfaceTuSupport, &presentationSupport);
            if (presentationSupport) {
                indices.presentFamily = i;
            }
        }
    }

    return indices;
}

bool hasAllRequiredExtensions(const std::vector<VkExtensionProperties>& availableExtensions) {
    for (const auto& requiredExtension : requiredExtensions) {
        auto found = false;
        for (const auto& availableExtension : availableExtensions) {
            if (strcmp(availableExtension.extensionName, requiredExtension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surfaceTuSupport) {
    return getQueueFamilies(device, surfaceTuSupport).isValid()
        && hasAllRequiredExtensions(getAvailableExtensions(device))
        && getSwapChainDetails(device, surfaceTuSupport).isValid();
}

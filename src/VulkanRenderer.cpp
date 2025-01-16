#include "VulkanRenderer.h"
#include "utils.h"
#include <stdexcept>
#include <tuple>

VulkanRenderer VulkanRenderer::create(GLFWwindow *window) {
    auto instance = createInstance();
    auto [device, queues] = getDevice(instance);
    return {
        window, instance, device, queues
    };
}

VulkanRenderer::VulkanRenderer(const GLFWwindow *window, VkInstance instance, const Device device, const QueueFamilyIndices queues)
    : window(window), instance(instance), device(device), queues(queues) {
}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(instance, nullptr);
    vkDestroyDevice(device.logicalDevice, nullptr);
}

std::vector<const char *> VulkanRenderer::getExtensions() {
    auto extensions = std::vector<const char *>();
    uint32_t glfwExtensionsCount;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    for (auto i = 0u; i < glfwExtensionsCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

VkApplicationInfo VulkanRenderer::getAppInfo() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    return appInfo;
}

VkInstance VulkanRenderer::createInstance() {
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

VkPhysicalDevice VulkanRenderer::getPhysicalDevice(VkInstance instance) {
    std::vector<VkPhysicalDevice> devices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan devices found");
    }
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device: devices) {
        if (isDeviceSuitable(device)) {
            return device;
        }
    }

    throw std::runtime_error("No suitable Vulkan devices found");
}

VkDevice VulkanRenderer::createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices queues) {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    auto [graphicsFamily] = queues;
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount = 1;
    constexpr float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = 0;
    deviceCreateInfo.ppEnabledExtensionNames = nullptr;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice logicalDevice;
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

    return logicalDevice;
}

std::tuple<VulkanRenderer::Device, VulkanRenderer::QueueFamilyIndices> VulkanRenderer::getDevice(VkInstance instance) {
    const auto physicalDevice = getPhysicalDevice(instance);
    const auto queues = getQueueFamilies(physicalDevice);
    const auto logicalDevice = createLogicalDevice(physicalDevice, queues);
    const Device device = {physicalDevice, logicalDevice};
    return {device, queues};
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    auto queueFamilyCount = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (auto i = 0; i < queueFamilyCount && !indices.isValid(); i++) {
        const auto queueFamily = queueFamilies[i];
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
    }

    return indices;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    auto indices = getQueueFamilies(device);
    return indices.isValid();
}

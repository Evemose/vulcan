#include "VulkanRenderer.h"
#include "utils.h"
#include <stdexcept>

VulkanRenderer VulkanRenderer::create(GLFWwindow *window) {
    auto instance = createInstance();
    auto device = getDevice(instance);
    return {
        window, instance, device
    };
}

VulkanRenderer::VulkanRenderer(const GLFWwindow *window, const VkInstance instance, const Device device)
    : window(window), instance(instance), device(device) {
}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(instance, nullptr);
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

VkPhysicalDevice VulkanRenderer::get_physical_device(VkInstance instance) {
    std::vector<VkPhysicalDevice> devices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan devices found");
    }
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    return devices[0];
}

VulkanRenderer::Device VulkanRenderer::getDevice(VkInstance instance) {
    auto physicalDevice = get_physical_device(instance);
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

}

#include "VulkanRenderer.h"

#include <cstring>
#include <ranges>
#include <set>

#include "utils.h"
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "swapchain_choices.h"

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
    auto swapChainAndMeta = createSwapChain(device, surface, window);
    return {
        window, instance, device, queues, surface, swapChainAndMeta
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

VulkanRenderer::VulkanRenderer(
    const GLFWwindow* window, VkInstance instance, Device device, QueueFamilyIndices queues,
    VkSurfaceKHR surface, SwapChainAndMetadata swapChainAndMetadata
): window(window), instance(instance), device(device), queues(queues), surface(surface),
   graphicsQueue(getGraphicsQueue(device.logicalDevice, queues.graphicsFamily)),
   presentQueue(getPresentQueue(device.logicalDevice, queues.presentFamily)),
   swapChainAndMetadata(std::move(swapChainAndMetadata)) {
}

VulkanRenderer::~VulkanRenderer() {
    for (const auto& swapChainImage : swapChainAndMetadata.swapChainImages) {
        vkDestroyImageView(device.logicalDevice, swapChainImage.imageView, nullptr);
    }
    vkDestroySwapchainKHR(device.logicalDevice, swapChainAndMetadata.swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
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

VkImageView VulkanRenderer::createImageView(
    VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlagBits vkImageAspectFlagBits
) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask = vkImageAspectFlagBits;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView));
    return imageView;
}

VulkanRenderer::SwapChainAndMetadata VulkanRenderer::createSwapChain(
    const Device device, const VkSurfaceKHR surface, GLFWwindow* window
) {
    const auto swapChainDetails = getSwapChainDetails(device.physicalDevice, surface);

    const auto bestSurfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
    const auto bestPresentMode = chooseSwapPresentMode(swapChainDetails.presentModes);
    const auto extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities, window);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.imageFormat = bestSurfaceFormat.format;
    createInfo.imageColorSpace = bestSurfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.presentMode = bestPresentMode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    const auto minSwapChainImages = std::min(
        swapChainDetails.surfaceCapabilities.minImageCount + 1,
        swapChainDetails.surfaceCapabilities.maxImageCount
    );
    createInfo.minImageCount = minSwapChainImages;

    auto queues = getQueueFamilies(device.physicalDevice, surface);

    if (queues.graphicsFamily != queues.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        const uint32_t queueFamilyIndices[] = {
            static_cast<uint32_t>(queues.graphicsFamily),
            static_cast<uint32_t>(queues.presentFamily)
        };
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    VK_CHECK(vkCreateSwapchainKHR(device.logicalDevice, &createInfo, nullptr, &swapChain));

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, nullptr);
    auto swapChainVkImages = std::vector<VkImage>(imageCount);
    vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, swapChainVkImages.data());

    auto mappedImages = std::vector<SwapChainImage>(imageCount);
    for (auto i = 0u; i < imageCount; i++) {
        auto& image = mappedImages[i];
        image.image = swapChainVkImages[i];
        image.imageView = createImageView(device.logicalDevice, image.image, bestSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    return {swapChain, bestSurfaceFormat.format, extent, mappedImages};
}

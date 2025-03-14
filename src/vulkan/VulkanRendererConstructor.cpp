#include "VulkanRendererConstructor.h"

#include <ranges>
#include <set>

#include "utils.h"

namespace enjine {
    const auto requiredExtensions = std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct SwapChainDetails {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    vk::UniqueInstance createInstance();
    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice &device, vk::SurfaceKHR surface);

    std::unique_ptr<VulkanRenderer> VulkanRendererConstructor::create(GLFWwindow *window) {
        this->window = window;
        instance = createInstance();
        surface = createSurface();
        physicalDevice = retrievePhysicalDevice();
        logicalDevice = createLogicalDevice();
        swapChainHandle = createSwapChain();

        return std::make_unique<VulkanRenderer>();
    }

    vk::UniqueSurfaceKHR VulkanRendererConstructor::createSurface() {
        VkSurfaceKHR glfwSurface;

        VK_CHECK(glfwCreateWindowSurface(instance.get(), window, nullptr, &glfwSurface));

        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(instance.get());

        return vk::UniqueSurfaceKHR(vk::SurfaceKHR(glfwSurface), deleter);
    }

    bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    vk::PhysicalDevice VulkanRendererConstructor::retrievePhysicalDevice() {
        auto devices = instance->enumeratePhysicalDevices();

        auto filtered = std::views::filter(devices, [this](const auto &device) {
            return isDeviceSuitable(device, surface.get());
        });

        if (filtered.begin() == filtered.end()) {
            throw std::runtime_error("No suitable device found");
        }

        return filtered.front();
    }

    std::vector<const char *> getExtensionsToEnable(vk::PhysicalDevice physicalDevice) {
        auto resultingExtensions = std::vector<const char *>();
        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        for (const auto &extension: requiredExtensions) {
            resultingExtensions.push_back(extension);
        }

        if (std::ranges::find_if(availableExtensions, [](const auto &extension) {
            return strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0;
        }) != availableExtensions.end()) {
            resultingExtensions.emplace_back("VK_KHR_portability_subset");
        }

        return resultingExtensions;
    }

    struct QueueHandles {
        QueueHandle graphicsQueue = QueueHandle{-1, nullptr};
        QueueHandle presentQueue = QueueHandle{-1, nullptr};

        [[nodiscard]] bool areIndicesValid() const {
            return graphicsQueue.index >= 0 && presentQueue.index >= 0;
        }
    };

    QueueHandles getQueueHandles(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        QueueHandles queueHandles;

        auto queueFamilyProperties = device.getQueueFamilyProperties();

        for (auto i = 0; i < queueFamilyProperties.size(); i++) {
            const auto &queueFamily = queueFamilyProperties[i];
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                queueHandles.graphicsQueue.index = i;
            }

            if (device.getSurfaceSupportKHR(i, surface)) {
                queueHandles.presentQueue.index = i;
            }

            if (queueHandles.graphicsQueue.index >= 0 && queueHandles.presentQueue.index >= 0) {
                break;
            }
        }

        return queueHandles;
    }

    std::vector<VkDeviceQueueCreateInfo> makeQueueCreateInfos(const QueueHandles & queueHandles) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        for (const auto &queueHandle: {queueHandles.graphicsQueue, queueHandles.presentQueue}) {
            if (uniqueQueueFamilies.contains(queueHandle.index)) {
                continue;
            }
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueHandle.index;
            queueCreateInfo.queueCount = 1;
            constexpr float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
            uniqueQueueFamilies.insert(queueHandles.graphicsQueue.index);
        }

        return queueCreateInfos;
    }

    vk::UniqueDevice VulkanRendererConstructor::createLogicalDevice() const {
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;

        auto enabledExtensions = getExtensionsToEnable(physicalDevice);
        createInfo.enabledExtensionCount = enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();

        auto features = VkPhysicalDeviceFeatures{};
        createInfo.pEnabledFeatures = &features;

        auto queueCrateInfos = makeQueueCreateInfos(getQueueHandles(physicalDevice, surface.get()));
        createInfo.queueCreateInfoCount = queueCrateInfos.size();
        createInfo.pQueueCreateInfos = queueCrateInfos.data();

        return physicalDevice.createDeviceUnique(createInfo);
    }

    SwapChainHandle VulkanRendererConstructor::createSwapChain() {
        auto swapChainDetails = getSwapChainDetails(physicalDevice, surface.get());
    }

    bool hasRequiredExtensions(const vk::PhysicalDevice &device) {
        auto availableExtensions = device.enumerateDeviceExtensionProperties();

        for (const auto &requiredExtension: requiredExtensions) {
            auto found = false;
            for (const auto &availableExtension: availableExtensions) {
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

    bool hasRequiredQueueFamilies(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        return getQueueHandles(device, surface).areIndicesValid();
    }

    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        SwapChainDetails details;

        details.surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);
        if (details.surfaceCapabilities.maxImageCount == 0) {
            details.surfaceCapabilities.maxImageCount = UINT32_MAX;
        }

        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);

        return details;
    }

    bool hasValidSwapChain(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        auto details = getSwapChainDetails(device, surface);
        return !details.formats.empty() && !details.presentModes.empty();
    }

    bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        return hasRequiredExtensions(device)
               && hasRequiredQueueFamilies(device, surface)
               && hasValidSwapChain(device, surface);
    }


    std::vector<const char *> getRequiredExtensions();

    VkApplicationInfo getAppInfo();

    vk::UniqueInstance createInstance() {
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.flags = 0;

        auto appInfo = getAppInfo();
        info.pApplicationInfo = &appInfo;

        const char *layers[] = {
            "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_profiles"
        };
        info.enabledLayerCount = 2;
        info.ppEnabledLayerNames = layers;

        auto extensions = getRequiredExtensions();
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();

        return vk::createInstanceUnique(info);
    }

    VkApplicationInfo getAppInfo() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Test";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        return appInfo;
    }

    std::vector<const char *> getRequiredExtensions() {
        auto extensions = std::vector<const char *>();
        uint32_t glfwExtensionCount;

        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (auto i = 0u; i < glfwExtensionCount; i++) {
            extensions.push_back(glfwExtensions[i]);
        }

        return extensions;
    }
}

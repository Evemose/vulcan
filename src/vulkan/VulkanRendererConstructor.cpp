#include "VulkanRendererConstructor.h"

#include <ranges>
#include <set>
#include <unordered_set>

#include "utils.h"

std::unordered_set preferableFormats = {
    vk::Format::eB8G8R8Unorm,
    vk::Format::eR8G8B8Unorm
};

auto preferableColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

namespace enjine {
    const auto requiredExtensions = std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct SwapChainDetails {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueHandles {
        QueueHandle graphicsQueue = QueueHandle{-1, nullptr};
        QueueHandle presentQueue = QueueHandle{-1, nullptr};

        [[nodiscard]] bool areIndicesValid() const {
            return graphicsQueue.index >= 0 && presentQueue.index >= 0;
        }
    };

    vk::UniqueInstance createInstance();
    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice& device, vk::SurfaceKHR surface);
    QueueHandles getRawQueueHandles(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    void VulkanRendererConstructor::initQueues() {
        auto rawQueueHandles = getRawQueueHandles(physicalDevice, surface.get());
        graphicsQueue = QueueHandle{
            rawQueueHandles.graphicsQueue.index,
            logicalDevice->getQueue(rawQueueHandles.graphicsQueue.index, 0)
        };
        presentQueue = QueueHandle{
            rawQueueHandles.presentQueue.index,
            logicalDevice->getQueue(rawQueueHandles.presentQueue.index, 0)
        };
    }

    std::unique_ptr<VulkanRenderer> VulkanRendererConstructor::create(GLFWwindow* window) {
        this->window = window;
        instance = createInstance();
        surface = createSurface();
        physicalDevice = retrievePhysicalDevice();
        logicalDevice = createLogicalDevice();
        initQueues();
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

        auto filtered = std::views::filter(devices, [this](const auto& device) {
            return isDeviceSuitable(device, surface.get());
        });

        if (filtered.begin() == filtered.end()) {
            throw std::runtime_error("No suitable device found");
        }

        return filtered.front();
    }

    std::vector<const char*> getExtensionsToEnable(vk::PhysicalDevice physicalDevice) {
        auto resultingExtensions = std::vector<const char*>();

        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        for (const auto& extension : requiredExtensions) {
            resultingExtensions.push_back(extension);
        }

        if (std::ranges::find_if(availableExtensions, [](const auto& extension) {
            return strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0;
        }) != availableExtensions.end()) {
            resultingExtensions.emplace_back("VK_KHR_portability_subset");
        }

        return resultingExtensions;
    }

    QueueHandles getRawQueueHandles(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        QueueHandles queueHandles;

        auto queueFamilyProperties = device.getQueueFamilyProperties();

        for (auto i = 0; i < queueFamilyProperties.size(); i++) {
            const auto& queueFamily = queueFamilyProperties[i];
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

    std::vector<VkDeviceQueueCreateInfo> makeQueueCreateInfos(const QueueHandles& queueHandles) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        for (const auto& queueHandle : {queueHandles.graphicsQueue, queueHandles.presentQueue}) {
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

        auto queueCrateInfos = makeQueueCreateInfos(getRawQueueHandles(physicalDevice, surface.get()));
        createInfo.queueCreateInfoCount = queueCrateInfos.size();
        createInfo.pQueueCreateInfos = queueCrateInfos.data();

        return physicalDevice.createDeviceUnique(createInfo);
    }

    vk::SurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
            return {*preferableFormats.begin(), preferableColorSpace};
        }

        for (const auto& format : availableFormats) {
            if (preferableFormats.contains(format.format) && format.colorSpace == preferableColorSpace) {
                return format;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR chooseBestPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
        for (const auto& mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                return mode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities, GLFWwindow* window) {
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            return surfaceCapabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width,
                                  surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height,
                                   surfaceCapabilities.maxImageExtent.height);

        return extent;
    }

    vk::UniqueImageView createImageView(vk::Device device, vk::Image image, vk::Format format, vk::ImageAspectFlagBits eColor) {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eImageViewCreateInfo;
        createInfo.pNext = nullptr;
        createInfo.image = image;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = format;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        return device.createImageViewUnique(createInfo);
    }

    SwapChainHandle VulkanRendererConstructor::createSwapChain() {
        auto swapChainDetails = getSwapChainDetails(physicalDevice, surface.get());

        auto bestFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
        auto bestPresentMode = chooseBestPresentMode(swapChainDetails.presentModes);
        auto extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities, window);

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        createInfo.pNext = nullptr;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        createInfo.surface = surface.get();
        createInfo.clipped = VK_TRUE;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        createInfo.imageExtent = extent;
        createInfo.imageFormat = bestFormat.format;
        createInfo.imageColorSpace = bestFormat.colorSpace;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        createInfo.clipped = VK_TRUE;
        createInfo.presentMode = bestPresentMode;
        const auto minSwapChainImages = std::min(
            swapChainDetails.surfaceCapabilities.minImageCount + 1,
            swapChainDetails.surfaceCapabilities.maxImageCount
        );
        createInfo.minImageCount = minSwapChainImages;
        createInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;

        if (graphicsQueue.index != presentQueue.index) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            const uint32_t queueFamilyIndices[] = {
                static_cast<uint32_t>(graphicsQueue.index),
                static_cast<uint32_t>(presentQueue.index)
            };
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }

        auto swapChain = logicalDevice->createSwapchainKHRUnique(createInfo);
        std::vector<SwapChainImage> swapChainImages;
        for (
            auto rawSwapChainImages = logicalDevice->getSwapchainImagesKHR(swapChain.get());
            const auto& rawSwapChainImage : rawSwapChainImages
        ) {
            auto imageView = createImageView(
                logicalDevice.get(),
                rawSwapChainImage,
                bestFormat.format,
                vk::ImageAspectFlagBits::eColor
            );

            vk::UniqueImage image{rawSwapChainImage, logicalDevice.get()};

            swapChainImages.push_back({
                std::move(image),
                std::move(imageView)
            });
        }

        return {
            std::move(swapChain),
            bestFormat.format,
            extent,
            std::move(swapChainImages)
        };
    }

    bool hasRequiredExtensions(const vk::PhysicalDevice& device) {
        auto availableExtensions = device.enumerateDeviceExtensionProperties();

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

    bool hasRequiredQueueFamilies(const vk::PhysicalDevice& device, vk::SurfaceKHR surface) {
        return getRawQueueHandles(device, surface).areIndicesValid();
    }

    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice& device, vk::SurfaceKHR surface) {
        SwapChainDetails details;

        details.surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);
        if (details.surfaceCapabilities.maxImageCount == 0) {
            details.surfaceCapabilities.maxImageCount = UINT32_MAX;
        }

        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);

        return details;
    }

    bool hasValidSwapChain(const vk::PhysicalDevice& device, vk::SurfaceKHR surface) {
        auto details = getSwapChainDetails(device, surface);
        return !details.formats.empty() && !details.presentModes.empty();
    }

    bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        return hasRequiredExtensions(device)
            && hasRequiredQueueFamilies(device, surface)
            && hasValidSwapChain(device, surface);
    }


    std::vector<const char*> getRequiredExtensions();

    VkApplicationInfo getAppInfo();

    vk::UniqueInstance createInstance() {
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.flags = 0;

        auto appInfo = getAppInfo();
        info.pApplicationInfo = &appInfo;

        const char* layers[] = {
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

    std::vector<const char*> getRequiredExtensions() {
        auto extensions = std::vector<const char*>();
        uint32_t glfwExtensionCount;

        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (auto i = 0u; i < glfwExtensionCount; i++) {
            extensions.push_back(glfwExtensions[i]);
        }

        return extensions;
    }
}

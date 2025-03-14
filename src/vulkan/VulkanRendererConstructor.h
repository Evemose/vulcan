#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>

#include "structs.h"
#include "VulkanRenderer.h"
#include "GLFW/glfw3.h"

namespace enjine {
    /**
     * A single use, stateful class used to construct VulkanRenderer instance.
     * Sharing this class between multiple threads is not safe.
     */
    class VulkanRendererConstructor {
    public:

        [[nodiscard]] std::unique_ptr<VulkanRenderer> create(GLFWwindow *window);

    private:
        bool alreadyUsed = false;

        // Vulkan structures
        GLFWwindow* window = nullptr;
        vk::UniqueInstance instance;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice logicalDevice;
        vk::UniqueSurfaceKHR surface;
        SwapChainHandle swapChainHandle{};
        QueueHandle graphicsQueue{};
        QueueHandle presentQueue{};
        std::vector<ImageResources> imageResources;
        std::vector<FrameSync> frameSyncs;

        [[nodiscard]] vk::UniqueSurfaceKHR createSurface();
        [[nodiscard]] vk::PhysicalDevice retrievePhysicalDevice();
        [[nodiscard]] vk::UniqueDevice createLogicalDevice() const;
        [[nodiscard]] SwapChainHandle createSwapChain();
    };
}

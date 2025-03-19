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
        [[nodiscard]] std::unique_ptr<VulkanRenderer> create(GLFWwindow *window, int maxFramesInFlight = 2);

    private:
        bool alreadyUsed = false;

        // Vulkan structures
        GLFWwindow *window = nullptr;
        vk::UniqueInstance instance;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice logicalDevice;
        vk::UniqueSurfaceKHR surface;
        SwapChainHandle swapChainHandle{};
        QueueHandle graphicsQueue{};
        QueueHandle presentQueue{};
        vk::UniqueRenderPass renderPass;
        vk::UniquePipeline graphicsPipeline;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniqueCommandPool commandPool;
        std::vector<ImageResources> imageResources;
        std::vector<FrameSync> frameSyncs;
        vk::UniqueDescriptorPool descriptorPool;
        vk::UniqueDescriptorSetLayout descriptorSetLayout;

        void createSurface();

        void retrievePhysicalDevice();

        void createLogicalDevice();

        void createSwapChain();

        void initQueues();

        void createRenderPass();

        void createGraphicsPipeline();

        void createImageResources();

        void createFramebuffers();

        void createCommandBuffers();

        void createDescriptorSetLayout();

        void createDescriptorPool();

        void allocateDescriptorSets();

        void createUniformBuffers();

        void bindDescriptorsToBuffers();

        void createDescriptorSets();

        void createFrameSyncs(int maxFramesInFlight);

        vk::UniqueShaderModule createShaderModule(const std::vector<char> &code);
    };
}

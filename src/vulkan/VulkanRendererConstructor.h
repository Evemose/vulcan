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
        [[nodiscard]] std::unique_ptr<VulkanRenderer> create(GLFWwindow *window, int maxObjectsPerFrame = 100000, int maxFramesInFlight = 2);

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
        std::unique_ptr<vk::PushConstantRange> pushConstantRange = std::make_unique<vk::PushConstantRange>();

        void createSurface();

        void retrievePhysicalDevice();

        void createLogicalDevice();

        void createSwapChain();

        void initQueues();

        void createRenderPass();

        void createGraphicsPipeline();

        void createPushConstantRange() const;

        void createImageResources(int maxObjectsPerFrame);

        void createFramebuffers();

        void createCommandBuffers();

        void createDescriptorSetLayout();

        void createDescriptorPool();

        void allocateDescriptorSets();

        void createViewProjectionBuffer(ImageResources &resource);

        void createUniformBuffers();

        void bindDescriptorsToBuffers();

        void createDescriptorSets();

        void createFrameSyncs(int maxFramesInFlight);

        vk::UniqueShaderModule createShaderModule(const std::vector<char> &code);
    };
}

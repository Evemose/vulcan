#pragma once
#include <memory>

#include "GLFW/glfw3.h"
#include <vulkan/vulkan.hpp>

namespace enjine {
    struct Model;

    struct QueueHandle {
        int index;
        vk::Queue queue;
    };

    struct ImageResources {
        vk::UniqueFramebuffer framebuffer;
        vk::UniqueCommandBuffer commandBuffer;
        vk::UniqueBuffer viewProjectionUniformBuffer;
        vk::UniqueDeviceMemory viewProjectionUniformBufferMemory;
        vk::UniqueDescriptorSet descriptorSet;
    };

    struct FrameSync {
        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderFinishedSemaphore;
        vk::UniqueFence inFlightFence;
    };

    struct SwapChainImage {
        vk::Image image;
        vk::UniqueImageView imageView;
    };

    struct SwapChainHandle {
        vk::UniqueSwapchainKHR swapChain;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        std::vector<SwapChainImage> swapChainImages;
    };

    struct Devices {
        vk::PhysicalDevice physicalDevice;
        vk::Device logicalDevice;
    };

    struct RendererResources {
        GLFWwindow* window;
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
        std::unique_ptr<vk::PushConstantRange> pushConstantRange;
    };
}

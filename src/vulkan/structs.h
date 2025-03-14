#pragma once

namespace enjine {
    struct QueueHandle {
        int index;
        vk::Queue queue;
    };

    struct ImageResources {
        vk::UniqueFramebuffer framebuffer;
        vk::UniqueCommandBuffer commandBuffer;
        vk::UniqueBuffer uniformBuffer;
        vk::UniqueDeviceMemory uniformBufferMemory;
        vk::UniqueDescriptorSet descriptorSet;
    };

    struct FrameSync {
        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderFinishedSemaphore;
        vk::UniqueFence inFlightFence;
    };

    struct SwapChainHandle {
        vk::UniqueSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<vk::UniqueImage> swapChainImages;
    };
}

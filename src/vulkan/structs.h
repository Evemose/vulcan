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

    struct SwapChainImage {
        vk::UniqueImage image;
        vk::UniqueImageView imageView;
    };

    struct SwapChainHandle {
        vk::UniqueSwapchainKHR swapChain;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        std::vector<SwapChainImage> swapChainImages;
    };
}

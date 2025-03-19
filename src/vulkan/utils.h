#pragma once
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

// Macros remain in the header.
#ifndef VK_CHECK
#define VK_CHECK(f)                                                                  \
do {                                                                                 \
    VkResult result = (f);                                                           \
    if (result != VK_SUCCESS) {                                                      \
        throw std::runtime_error(                                                    \
            std::string("Vulkan call '") + #f + "' failed with error code " +        \
            std::to_string(result));                                                 \
    }                                                                                \
} while(0)
#endif

#ifndef VK_CHECK_CPP
#define VK_CHECK_CPP(f)                                                              \
do {                                                                                 \
    vk::Result result = (f);                                                         \
    if (result != vk::Result::eSuccess) {                                            \
        throw std::runtime_error(                                                    \
            std::string("Vulkan call '") + #f + "' failed with error code " +        \
            vk::to_string(result));                                                  \
    }                                                                                \
} while(0)
#endif

namespace enjine {
    struct Devices;

    struct BufferCreateResult {
        vk::UniqueBuffer buffer;
        vk::UniqueDeviceMemory memory;
    };

    uint32_t findMemoryTypeIdx(vk::PhysicalDevice physicalDevice, uint32_t memoryTypeBits,
                               vk::MemoryPropertyFlags flags);

    BufferCreateResult createBuffer(const Devices &devices, vk::DeviceSize size,
                                    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    void fillBuffer(const Devices &devices, vk::DeviceMemory bufferMemory,
                    vk::DeviceSize size, const void *data);

    void copyBuffer(const Devices &devices, vk::CommandPool commandPool,
                    vk::Queue transferQueue, vk::Buffer srcBuffer,
                    vk::Buffer dstBuffer, vk::DeviceSize size);

    template<typename T>
    BufferCreateResult createGpuLocalBuffer(
        const Devices &devices,
        const std::vector<T> &data,
        vk::BufferUsageFlags usage,
        vk::CommandPool commandPool,
        vk::Queue transferQueue
    ) {
        auto srcResult = createBuffer(
            devices,
            sizeof(T) * data.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );
        fillBuffer(devices, srcResult.memory.get(), sizeof(T) * data.size(), data.data());

        auto dstResult = createBuffer(
            devices,
            sizeof(T) * data.size(),
            vk::BufferUsageFlagBits::eTransferDst | usage,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        copyBuffer(devices, commandPool, transferQueue,
                   srcResult.buffer.get(), dstResult.buffer.get(),
                   sizeof(T) * data.size());

        return dstResult;
    }
}

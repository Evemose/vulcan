#include "utils.h"
#include <cstring>

#include "structs.h"

namespace enjine {

    uint32_t findMemoryTypeIdx(vk::PhysicalDevice physicalDevice, uint32_t memoryTypeBits,
                               vk::MemoryPropertyFlags flags) {
        auto memoryProperties = physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if (memoryTypeBits & (1 << i) &&
                (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    createViewProjectionBuffers createBuffer(const Devices &devices, vk::DeviceSize size,
                                    vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
        vk::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = vk::StructureType::eBufferCreateInfo;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

        auto buffer = devices.logicalDevice.createBufferUnique(bufferCreateInfo);

        auto memRequirements = devices.logicalDevice.getBufferMemoryRequirements(buffer.get());

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryTypeIdx(devices.physicalDevice, memRequirements.memoryTypeBits,
                                                      properties);

        auto memory = devices.logicalDevice.allocateMemoryUnique(allocInfo);
        devices.logicalDevice.bindBufferMemory(buffer.get(), memory.get(), 0);

        return {std::move(buffer), std::move(memory)};
    }

    void fillBuffer(const Devices &devices, vk::DeviceMemory bufferMemory,
                    vk::DeviceSize size, const void *data) {
        void *mappedData;
        VK_CHECK_CPP(devices.logicalDevice.mapMemory(bufferMemory, 0, size, {}, &mappedData));
        std::memcpy(mappedData, data, size);
        devices.logicalDevice.unmapMemory(bufferMemory);
    }

    void copyBuffer(const Devices &devices, vk::CommandPool commandPool,
                    vk::Queue transferQueue, vk::Buffer srcBuffer,
                    vk::Buffer dstBuffer, vk::DeviceSize size) {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        auto commandBuffer = devices.logicalDevice.allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        commandBuffer.begin(beginInfo);

        vk::BufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
        commandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        transferQueue.submit(submitInfo, nullptr);
        transferQueue.waitIdle();

        devices.logicalDevice.freeCommandBuffers(commandPool, commandBuffer);
    }

    uint32_t getAlignmentSizeForType(vk::PhysicalDevice physicalDevice, uint32_t typeSize) {
        auto minAlignment = physicalDevice.getProperties().limits.minUniformBufferOffsetAlignment;
        return (typeSize + minAlignment - 1) & ~(minAlignment - 1);
    }
}


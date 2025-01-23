#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>

#ifndef VK_CHECK
#define VK_CHECK(f)                                                                  \
do {                                                                             \
VkResult result = (f);                                                       \
if (result != VK_SUCCESS) {                                                  \
throw std::runtime_error(                                               \
std::string("Vulkan call '") + #f + "' failed with error code " +    \
std::to_string(result));                                             \
}                                                                            \
} while(0)
#endif

static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0);
    file.read(buffer.data(), static_cast<long>(buffer.size()));
    file.close();
    return buffer;
}

const std::string GLSL_RESOURCE_PREFIX = "../build/shaders/";

static std::vector<char> readGlslResource(const std::string &resourceName) {
    return readFile(GLSL_RESOURCE_PREFIX + resourceName + ".spv");
}

typedef struct {
    glm::vec3 pos;
    glm::vec3 color;
} Vertex;

static uint32_t findMemoryTypeIdx(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits, uint32_t flags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (auto i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memoryTypeBits & 1 << i && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

static std::tuple<VkBuffer, VkDeviceMemory> createBuffer(
    VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties
) {
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryTypeIdx(physicalDevice, memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));
    vkBindBufferMemory(device, buffer, bufferMemory, 0);

    return std::make_tuple(buffer, bufferMemory);
}

static void fillBuffer(VkDevice device, VkDeviceMemory bufferMemory, VkDeviceSize size, const void *data) {
    void *mappedData;
    vkMapMemory(device, bufferMemory, 0, size, 0, &mappedData);
    std::memcpy(mappedData, data, size);
    vkUnmapMemory(device, bufferMemory);
}

static void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue transferQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK(vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(transferQueue));

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

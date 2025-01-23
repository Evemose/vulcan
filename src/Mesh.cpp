#include "Mesh.h"

#include <cstring>

Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<Vertex> &vertices, VkBuffer vertexBuffer,
           VkDeviceMemory vertexBufferMemory)
    : vertexCount(static_cast<int>(vertices.size())),
      physicalDevice(physicalDevice),
      device(device),
      vertexBuffer(vertexBuffer),
      vertexBufferMemory(vertexBufferMemory) {
}

int Mesh::getVertexCount() const {
    return vertexCount;
}

VkBuffer Mesh::getVertexBuffer() const {
    return vertexBuffer;
}

VkDeviceMemory Mesh::getVertexBufferMemory() const {
    return vertexBufferMemory;
}

Mesh::~Mesh() {
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

Mesh Mesh::create(const std::vector<Vertex> &vertices, VkDevice device, VkPhysicalDevice physicalDevice) {
    const auto [buffer, memory] = createVertexBuffer(vertices, device, physicalDevice);
    return {
        physicalDevice, device, vertices, buffer, memory
    };
}

Mesh::Mesh(Mesh && other) noexcept {
    vertexCount = other.vertexCount;
    physicalDevice = other.physicalDevice;
    device = other.device;
    vertexBuffer = other.vertexBuffer;
    vertexBufferMemory = other.vertexBufferMemory;

    // Reset the other object
    other.vertexBuffer = VK_NULL_HANDLE;
    other.vertexBufferMemory = VK_NULL_HANDLE;
}

uint32_t findMemoryTypeIdx(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits, int flags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (auto i = 0; i < memProperties.memoryTypeCount; i++) {
        if (memoryTypeBits & 1 << i && (memProperties.memoryTypes[i].propertyFlags & flags) == flags) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

std::tuple<VkBuffer, VkDeviceMemory> Mesh::createVertexBuffer(
    const std::vector<Vertex> &vertices, VkDevice device, VkPhysicalDevice physicalDevice
) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer vertexBuffer;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryTypeIdx(
        physicalDevice,
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    VkDeviceMemory vertexBufferMemory;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory));

    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    void* mappedMemory;
    vkMapMemory(device, vertexBufferMemory, 0, bufferInfo.size, 0, &mappedMemory);
    memcpy(mappedMemory, vertices.data(), bufferInfo.size);
    vkUnmapMemory(device, vertexBufferMemory);

    return {vertexBuffer, vertexBufferMemory};
}

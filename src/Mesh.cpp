#include "Mesh.h"

Mesh::Mesh(VkDevice device, Buffer &vertexBuffer, Buffer &indexBuffer): device(device) {
    this->vertexBuffer = std::move(vertexBuffer);
    this->indexBuffer = std::move(indexBuffer);
}

int Mesh::getVertexCount() const {
    return vertexBuffer.elementCount;
}

VkBuffer Mesh::getVertexBuffer() const {
    return vertexBuffer.buffer;
}

VkDeviceMemory Mesh::getVertexBufferMemory() const {
    return vertexBuffer.memory;
}

Mesh::~Mesh() {
    vertexBuffer.cleanup(device);
    indexBuffer.cleanup(device);
}

VkBuffer Mesh::getIndexBuffer() const {
    return indexBuffer.buffer;
}

uint32_t Mesh::getIndexCount() const {
    return indexBuffer.elementCount;
}

Mesh::Buffer Mesh::createVertexBuffer(
    const std::vector<Vertex> &vertices, VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool, VkQueue transferQueue
) {
    const auto [srcBuffer, srcMem] = createBuffer(
        device,
        physicalDevice,
        sizeof(Vertex) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    fillBuffer(device, srcMem, sizeof(Vertex) * vertices.size(), vertices.data());

    const auto [dstBuffer, dstMem] = createBuffer(
        device,
        physicalDevice,
        sizeof(Vertex) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    copyBuffer(device, commandPool, transferQueue, srcBuffer, dstBuffer, sizeof(Vertex) * vertices.size());

    vkDestroyBuffer(device, srcBuffer, nullptr);
    vkFreeMemory(device, srcMem, nullptr);

    return {static_cast<int>(vertices.size()), dstBuffer, dstMem};
}

Mesh::Buffer Mesh::createIndexBuffer(const std::vector<uint32_t> &indices, VkDevice device,
                                     VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
                                     VkQueue transferQueue) {
    const auto [srcBuffer, srcMem] = createBuffer(
        device,
        physicalDevice,
        sizeof(uint32_t) * indices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    fillBuffer(device, srcMem, sizeof(uint32_t) * indices.size(), indices.data());

    const auto [dstBuffer, dstMem] = createBuffer(
        device,
        physicalDevice,
        sizeof(uint32_t) * indices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    copyBuffer(device, commandPool, transferQueue, srcBuffer, dstBuffer, sizeof(uint32_t) * indices.size());
    vkDestroyBuffer(device, srcBuffer, nullptr);
    vkFreeMemory(device, srcMem, nullptr);

    return {static_cast<int>(indices.size()), dstBuffer, dstMem};
}

Mesh Mesh::create(
    const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
    VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue transferQueue
) {
    auto vertexBuffer = createVertexBuffer(vertices, device, physicalDevice, commandPool, transferQueue);
    auto indexBuffer = createIndexBuffer(indices, device, physicalDevice, commandPool, transferQueue);

    return {
        device, vertexBuffer, indexBuffer
    };
}

Mesh::Mesh(Mesh &&other) noexcept {
    this->device = other.device;
    this->vertexBuffer = std::move(other.vertexBuffer);
    this->indexBuffer = std::move(other.indexBuffer);
}

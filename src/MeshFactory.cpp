#include "MeshFactory.h"

MeshFactory::MeshFactory(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue transferQueue):
    physicalDevice(physicalDevice),
    device(device), commandPool(commandPool), transferQueue(transferQueue) {
}

Mesh MeshFactory::createMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) const {
    return Mesh::create(vertices, indices, device, physicalDevice, commandPool, transferQueue);
}

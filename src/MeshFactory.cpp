#include "MeshFactory.h"

MeshFactory::MeshFactory(VkPhysicalDevice physicalDevice, VkDevice device) :
    physicalDevice(physicalDevice),
    device(device) {
}

std::unique_ptr<Mesh> MeshFactory::createMesh(const std::vector<Vertex> &vertices) const {
    auto tmp = Mesh::create(vertices, device, physicalDevice);
    return std::make_unique<Mesh>(std::move(tmp));
}

#ifndef MESHFACTORY_H
#define MESHFACTORY_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Mesh.h"


class MeshFactory {
public:
    MeshFactory(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue transferQueue);
    [[nodiscard]] Mesh createMesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices) const;
private:
    const VkPhysicalDevice physicalDevice;
    const VkDevice device;
    const VkCommandPool commandPool;
    const VkQueue transferQueue;
};


#endif //MESHFACTORY_H

#ifndef MESHFACTORY_H
#define MESHFACTORY_H
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Mesh.h"


class MeshFactory {
public:
    MeshFactory(VkPhysicalDevice physicalDevice, VkDevice device);
    [[nodiscard]] std::unique_ptr<Mesh> createMesh(const std::vector<Vertex> &vertices) const;
private:
    const VkPhysicalDevice physicalDevice;
    const VkDevice device;
};


#endif //MESHFACTORY_H

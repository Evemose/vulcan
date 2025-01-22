#ifndef MESH_H
#define MESH_H
#include <vector>
#include <vulkan/vulkan_core.h>
#include <tuple>

#include "utils.h"


class Mesh {
public:
    [[nodiscard]] int getVertexCount() const;

    [[nodiscard]] VkBuffer getVertexBuffer() const;

    [[nodiscard]] VkDeviceMemory getVertexBufferMemory() const;

    ~Mesh();

    static Mesh create(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<Vertex>& vertices);


private:
    const int vertexCount;
    const VkPhysicalDevice physicalDevice;
    const VkDevice device;
    const VkBuffer vertexBuffer;
    const VkDeviceMemory vertexBufferMemory;

    static std::tuple<VkBuffer, VkDeviceMemory> createVertexBuffer(const std::vector<Vertex> &vertices, VkDevice device, VkPhysicalDevice physicalDevice);

    Mesh(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<Vertex>& vertices, VkBuffer vertexBuffer, VkDeviceMemory vertexBufferMemory);
};


#endif //MESH_H

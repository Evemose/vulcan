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

    static Mesh create(const std::vector<Vertex>& vertices, VkDevice device, VkPhysicalDevice physicalDevice);

    Mesh(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept ;

private:
    int vertexCount;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    static std::tuple<VkBuffer, VkDeviceMemory> createVertexBuffer(const std::vector<Vertex> &vertices, VkDevice device, VkPhysicalDevice physicalDevice);

    Mesh(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<Vertex>& vertices, VkBuffer vertexBuffer, VkDeviceMemory vertexBufferMemory);
};


#endif //MESH_H

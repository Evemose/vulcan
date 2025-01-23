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

    [[nodiscard]] VkBuffer getIndexBuffer() const;

    [[nodiscard]] uint32_t getIndexCount() const;

    static Mesh create(
        const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
        VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
        VkQueue transferQueue
    );

    Mesh(const Mesh &) = delete;

    Mesh(Mesh &&other) noexcept;

private:
    typedef struct Buffer {
        int elementCount;
        VkBuffer buffer;
        VkDeviceMemory memory;

        void cleanup(VkDevice device) const {
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }

        void invalidate() {
            elementCount = 0;
            buffer = VK_NULL_HANDLE;
            memory = VK_NULL_HANDLE;
        }

        Buffer &operator=(Buffer &&other) noexcept {
            elementCount = other.elementCount;
            buffer = other.buffer;
            memory = other.memory;
            other.invalidate();

            return *this;
        }
    } Buffer;

    VkDevice device;
    Buffer vertexBuffer{};
    Buffer indexBuffer{};

    Mesh(VkDevice device, Buffer &vertexBuffer, Buffer &indexBuffer);

    static Buffer createVertexBuffer(
        const std::vector<Vertex> &vertices, VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool, VkQueue transferQueue);

    static Buffer createIndexBuffer(
        const std::vector<uint32_t> &indices, VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool, VkQueue transferQueue);
};


#endif //MESH_H

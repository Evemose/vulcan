#pragma once
#include <vulkan/vulkan.hpp>
#include "structs.h"
#include "utils.h"
#include "../Mesh.h"

namespace enjine {

    struct MeshBufferResources {
        Devices devices;
        vk::CommandPool commandPool;
        vk::Queue transferQueue;
    };

    class MeshBuffers {
    public:
        MeshBuffers(const MeshBufferResources& resources, const Mesh& mesh);
        vk::Buffer getVertexBuffer();
        vk::Buffer getIndexBuffer();

    private:
        struct CachedBufferInfo {
            vk::UniqueBuffer buffer;
            vk::UniqueDeviceMemory memory;
            bool initialized = false;
        };

        const Mesh mesh;
        const MeshBufferResources resources;
        CachedBufferInfo vertexBuffer{};
        CachedBufferInfo indexBuffer{};

        createViewProjectionBuffers createVertexBuffer() const;
        createViewProjectionBuffers createIndexBuffer() const;
    };
}

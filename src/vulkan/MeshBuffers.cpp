#include "MeshBuffers.h"

#include "utils.h"

namespace enjine {
    MeshBuffers::MeshBuffers(
        const MeshBufferResources &resources,
        const Mesh &mesh
    ) : mesh(mesh), resources(resources) {
    }

    vk::Buffer MeshBuffers::getVertexBuffer() {
        if (!vertexBuffer.initialized) {
            auto result = createVertexBuffer();
            vertexBuffer.buffer = std::move(result.buffer);
            vertexBuffer.memory = std::move(result.memory);
            vertexBuffer.initialized = true;
        }
        return vertexBuffer.buffer.get();
    }

    vk::Buffer MeshBuffers::getIndexBuffer() {
        if (!indexBuffer.initialized) {
            auto result = createIndexBuffer();
            indexBuffer.buffer = std::move(result.buffer);
            indexBuffer.memory = std::move(result.memory);
            indexBuffer.initialized = true;
        }
        return indexBuffer.buffer.get();
    }

    createViewProjectionBuffers MeshBuffers::createVertexBuffer() const {
        return createGpuLocalBuffer(
            resources.devices,
            mesh.vertices,
            vk::BufferUsageFlagBits::eVertexBuffer,
            resources.commandPool,
            resources.transferQueue
        );
    }

    createViewProjectionBuffers MeshBuffers::createIndexBuffer() const {
        return createGpuLocalBuffer(
            resources.devices,
            mesh.indices,
            vk::BufferUsageFlagBits::eIndexBuffer,
            resources.commandPool,
            resources.transferQueue
        );
    }
}

#pragma once
#include <memory>

#include "MeshBuffers.h"
#include "structs.h"
#include "../IRenderer.h"

namespace enjine {
    class VulkanRenderer final : IRenderer {
    public:

        void render(const std::vector<RenderObject> &meshes, ViewProjection viewProjection) override;

        explicit VulkanRenderer(RendererResources &&resources, int maxObjectsPerFrame)
            : resources(std::move(resources)),
              maxFramesInFlight(this->resources.frameSyncs.size()),
              maxObjectsPerFrame(maxObjectsPerFrame),
              inUseBuffersByImageIndex(initInUseBuffersMap()) {
        }

        [[nodiscard]] Extent getExtent() const override;

        ~VulkanRenderer() override;

    private:
        const RendererResources resources;
        const size_t maxFramesInFlight;
        const int maxObjectsPerFrame;
        std::vector<std::vector<MeshBuffers> > inUseBuffersByImageIndex;
        int currentFrame = 0;

        void writeViewProjection(uint32_t imageIndex, const ViewProjection &viewProjection) const;

        void recordDrawCommand(uint32_t imageIndex, const std::vector<RenderObject> &objects);

        void submitBuffer(uint32_t imageIndex) const;

        void present(uint32_t imageIndex) const;

        std::vector<std::vector<MeshBuffers> > initInUseBuffersMap() const;
    };
}

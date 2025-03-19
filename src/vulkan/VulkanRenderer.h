#pragma once
#include <memory>

#include "MeshBuffers.h"
#include "structs.h"
#include "../IRenderer.h"

namespace enjine {
    class VulkanRenderer final : IRenderer {
    public:
        void render(const std::vector<RenderObject> &meshes) override;

        explicit VulkanRenderer(RendererResources &&resources)
            : resources(std::move(resources)),
              maxFramesInFlight(this->resources.frameSyncs.size()),
              inUseBuffersByImageIndex(initInUseBuffersMap()) {
        }

    private:
        const RendererResources resources;
        const size_t maxFramesInFlight;
        std::vector<std::vector<MeshBuffers>> inUseBuffersByImageIndex;
        int currentFrame = 0;

        void recordDrawCommand(uint32_t imageIndex, const std::vector<RenderObject> &objects);

        void submitBuffer(uint32_t imageIndex) const;

        void present(uint32_t imageIndex) const;

        std::vector<std::vector<MeshBuffers>> initInUseBuffersMap() const;
    };
}

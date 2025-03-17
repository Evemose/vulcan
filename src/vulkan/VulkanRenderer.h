#pragma once
#include "structs.h"
#include "../IRenderer.h"

namespace enjine {
    class VulkanRenderer final : IRenderer {
    public:

        void render(const std::vector<RenderObject> &meshes) override;

        explicit VulkanRenderer(RendererResources &&resources) : resources(std::move(resources)),
                                                                 maxFramesInFlight(this->resources.frameSyncs.size()) {
        }

    private:
        const RendererResources resources;
        const size_t maxFramesInFlight;
        int currentFrame = 0;

        void recordDrawCommand(uint32_t imageIndex, const std::vector<RenderObject> &objects) const;
        void submitBuffer(uint32_t imageIndex) const;
        void present(uint32_t imageIndex) const;
    };
}

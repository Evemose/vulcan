#pragma once
#include "../IRenderer.h"

namespace enjine {
    class VulkanRenderer final : IRenderer {
    public:
        void render(const std::vector<RenderObject> &meshes) override;
    };
}

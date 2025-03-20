#pragma once
#include <vector>

#include "RenderObject.h"
#include "ViewProjection.h"

namespace enjine {

    struct Extent {
        uint32_t width;
        uint32_t height;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        virtual void render(const std::vector<RenderObject>& meshes, ViewProjection viewProjection) = 0;
        [[nodiscard]] virtual Extent getExtent() const = 0;
    };
}

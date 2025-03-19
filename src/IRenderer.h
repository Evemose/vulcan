#pragma once
#include <vector>

#include "RenderObject.h"
#include "ViewProjection.h"

namespace enjine {
    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        virtual void render(const std::vector<RenderObject>& meshes, ViewProjection viewProjection) = 0;
    };
}

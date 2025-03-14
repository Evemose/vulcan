#pragma once
#include "Mesh.h"

namespace enjine {

    struct RenderObject {
        Mesh mesh;
        glm::mat4 model{};
    };

}

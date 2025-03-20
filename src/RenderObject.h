#pragma once
#include "Mesh.h"

namespace enjine {

    struct Model {
        glm::mat4 model{};
    };

    struct RenderObject {
        Mesh mesh;
        Model model;
    };

}

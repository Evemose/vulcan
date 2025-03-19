#pragma once
#include <glm/glm.hpp>

namespace enjine {

    struct ViewProjection {
        glm::mat4 projection;
        glm::mat4 view;
    };

}

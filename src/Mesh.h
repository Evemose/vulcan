#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace enjine {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}
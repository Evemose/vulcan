#pragma once
#include <fstream>
#include <vector>

namespace enjine {
    inline std::vector<char> readFile(const std::string& path) {
        std::vector<char> result;
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file " + path);
        }

        auto fileSize = file.tellg();
        result.resize(fileSize);
        file.seekg(0);
        file.read(result.data(), fileSize);

        return std::move(result);
    }

    inline std::vector<char> readSpvResource(const std::string& resourceName) {
        return readFile("../build/shaders/" + resourceName + ".spv");
    }

}

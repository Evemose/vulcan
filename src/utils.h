#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>

// Usage example:
//    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
//
// If vkCreateInstance does not return VK_SUCCESS, the macro throws
// std::runtime_error with an error message.

#ifndef VK_CHECK
#define VK_CHECK(f)                                                                  \
do {                                                                             \
VkResult result = (f);                                                       \
if (result != VK_SUCCESS) {                                                  \
throw std::runtime_error(                                               \
std::string("Vulkan call '") + #f + "' failed with error code " +    \
std::to_string(result));                                             \
}                                                                            \
} while(0)
#endif

static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::vector<char> buffer(file.tellg());
    file.seekg(0);
    file.read(buffer.data(), static_cast<long>(buffer.size()));
    file.close();
    return buffer;
}

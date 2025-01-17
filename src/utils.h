#pragma once
#include <stdexcept>   // For std::runtime_error
#include <string>      // For std::to_string

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

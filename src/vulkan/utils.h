#pragma once
#include <stdexcept>

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

namespace enjine {
}
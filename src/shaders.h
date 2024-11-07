//
// Created by 4upa4 on 04.11.2024.
//

#ifndef SHADERS_H
#define SHADERS_H
#include <vector>
#include <vulkan/vulkan_core.h>

void createShaderModule(VkDevice& device, const std::vector<char>& code, VkShaderModule& shaderModule);

#endif //SHADERS_H

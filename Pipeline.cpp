#include "Pipeline.h"

#include "device.h"
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include "files.cpp"

Pipeline::Pipeline(Device& device, VkPipeline pipeline, VkShaderModule vertexShaderModule)
    : device(device), pipeline(pipeline), vertexShaderModule(vertexShaderModule) {}

Pipeline::~Pipeline() {
    vkDestroyPipeline(device.device(), pipeline, nullptr);
    vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(deviceField.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
}
#include "Pipeline.h"

#include "device.h"

Pipeline::Pipeline(Device& device, VkPipeline pipeline, VkShaderModule vertexShaderModule)
    : device(device), pipeline(pipeline), vertexShaderModule(vertexShaderModule) {
}

Pipeline::~Pipeline() {
    vkDestroyPipeline(device.device(), pipeline, nullptr);
    vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
}

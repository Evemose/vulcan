#include "PipelineBuilder.h"
#include <stdexcept>
#include <vector>

PipelineBuilder & PipelineBuilder::setVertexStage(const VkPipelineShaderStageCreateInfo &vertexStage) {
    this->vertexStage = vertexStage;
    return *this;
}

PipelineBuilder & PipelineBuilder::setFragmentStage(const VkPipelineShaderStageCreateInfo &fragmentStage) {
    this->fragmentStage = fragmentStage;
    return *this;
}

PipelineBuilder& PipelineBuilder::setRenderPass(VkRenderPass rp) {
    renderPass = rp;
    return *this;
}

PipelineBuilder& PipelineBuilder::setPipelineLayout(VkPipelineLayout pl) {
    pipelineLayout = pl;
    return *this;
}

VkPipeline PipelineBuilder::build(VkDevice device) const {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    if (vertexStage.has_value()) shaderStages.push_back(vertexStage.value());
    if (fragmentStage.has_value()) shaderStages.push_back(fragmentStage.value());

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.layout = pipelineLayout.value_or(defaultPipelineLayout);
    pipelineInfo.renderPass = renderPass.value_or(defaultRenderPass);

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    return pipeline;
}

void PipelineBuilder::setDefaultRenderPass(VkRenderPass rp) {
    defaultRenderPass = rp;
}

void PipelineBuilder::setDefaultPipelineLayout(VkPipelineLayout pl) {
    defaultPipelineLayout = pl;
}


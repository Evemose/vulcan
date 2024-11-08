#include "PipelineLayoutBuilder.h"

#include <stdexcept>

PipelineLayoutBuilder::PipelineLayoutBuilder() {
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    descriptorSetLayouts.push_back(descriptorSetLayout);
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::addPushConstantRange(const VkPushConstantRange& pushConstantRange) {
    pushConstantRanges.push_back(pushConstantRange);
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    return *this;
}

VkPipelineLayout PipelineLayoutBuilder::build(VkDevice device) const {
    auto pipelineLayout = VkPipelineLayout{};
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout.");
    }
    return pipelineLayout;
}

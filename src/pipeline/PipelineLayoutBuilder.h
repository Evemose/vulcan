#ifndef PIPELINELAYOUTBUILDER_H
#define PIPELINELAYOUTBUILDER_H
#include <vector>
#include <vulkan/vulkan_core.h>


class PipelineLayoutBuilder {
    public:
    PipelineLayoutBuilder();

    PipelineLayoutBuilder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    PipelineLayoutBuilder& addPushConstantRange(const VkPushConstantRange& pushConstantRange);

    [[nodiscard]] VkPipelineLayout build(VkDevice device) const;

private:
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkPushConstantRange> pushConstantRanges;
};



#endif //PIPELINELAYOUTBUILDER_H

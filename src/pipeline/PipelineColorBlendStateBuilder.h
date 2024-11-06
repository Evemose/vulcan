#ifndef PIPELINE_COLOR_BLEND_STATE_BUILDER_H
#define PIPELINE_COLOR_BLEND_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineColorBlendStateBuilder {
public:
    PipelineColorBlendStateBuilder();

    PipelineColorBlendStateBuilder& setBlendEnable(VkBool32 enable);
    PipelineColorBlendStateBuilder& setColorWriteMask(VkColorComponentFlags mask);

    [[nodiscard]] VkPipelineColorBlendStateCreateInfo build() const;

private:
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
};

#endif  // PIPELINE_COLOR_BLEND_STATE_BUILDER_H

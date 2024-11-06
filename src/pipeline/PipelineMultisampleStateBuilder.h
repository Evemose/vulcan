#ifndef PIPELINE_MULTISAMPLE_STATE_BUILDER_H
#define PIPELINE_MULTISAMPLE_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineMultisampleStateBuilder {
public:
    explicit PipelineMultisampleStateBuilder(VkSampleCountFlagBits samples);

    PipelineMultisampleStateBuilder& setSampleShadingEnable(VkBool32 enable);
    PipelineMultisampleStateBuilder& setMinSampleShading(float minSampleShading);

    [[nodiscard]] VkPipelineMultisampleStateCreateInfo build() const;

private:
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
};

#endif  // PIPELINE_MULTISAMPLE_STATE_BUILDER_H

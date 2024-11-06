#include "PipelineMultisampleStateBuilder.h"

PipelineMultisampleStateBuilder::PipelineMultisampleStateBuilder(VkSampleCountFlagBits samples) {
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.rasterizationSamples = samples;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.minSampleShading = 1.0f;
}

PipelineMultisampleStateBuilder& PipelineMultisampleStateBuilder::setSampleShadingEnable(VkBool32 enable) {
    multisampleInfo.sampleShadingEnable = enable;
    return *this;
}

PipelineMultisampleStateBuilder& PipelineMultisampleStateBuilder::setMinSampleShading(float minSampleShading) {
    multisampleInfo.minSampleShading = minSampleShading;
    return *this;
}

VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateBuilder::build() const {
    return multisampleInfo;
}

#ifndef PIPELINE_RASTERIZATION_STATE_BUILDER_H
#define PIPELINE_RASTERIZATION_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineRasterizationStateBuilder {
public:
    PipelineRasterizationStateBuilder(VkPolygonMode polygonMode, float lineWidth);

    PipelineRasterizationStateBuilder& setCullMode(VkCullModeFlags cullMode);
    PipelineRasterizationStateBuilder& setFrontFace(VkFrontFace frontFace);
    PipelineRasterizationStateBuilder& setDepthBiasEnable(VkBool32 enable);

    [[nodiscard]] VkPipelineRasterizationStateCreateInfo build() const;

private:
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
};

#endif  // PIPELINE_RASTERIZATION_STATE_BUILDER_H

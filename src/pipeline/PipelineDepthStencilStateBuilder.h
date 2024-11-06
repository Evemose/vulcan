#ifndef PIPELINE_DEPTH_STENCIL_STATE_BUILDER_H
#define PIPELINE_DEPTH_STENCIL_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineDepthStencilStateBuilder {
public:
    PipelineDepthStencilStateBuilder();

    PipelineDepthStencilStateBuilder& setDepthTestEnable(VkBool32 enable);
    PipelineDepthStencilStateBuilder& setDepthWriteEnable(VkBool32 enable);

    [[nodiscard]] VkPipelineDepthStencilStateCreateInfo build() const;

private:
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
};

#endif  // PIPELINE_DEPTH_STENCIL_STATE_BUILDER_H
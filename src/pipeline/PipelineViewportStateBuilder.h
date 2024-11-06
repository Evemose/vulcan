#ifndef PIPELINE_VIEWPORT_STATE_BUILDER_H
#define PIPELINE_VIEWPORT_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineViewportStateBuilder {
public:
    PipelineViewportStateBuilder(float width, float height);

    PipelineViewportStateBuilder& setViewport(float x, float y, float minDepth, float maxDepth);
    PipelineViewportStateBuilder& setScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height);

    [[nodiscard]] VkPipelineViewportStateCreateInfo build();

private:
    VkViewport viewport{};
    VkRect2D scissor{};
    VkPipelineViewportStateCreateInfo viewportInfo{};
};

#endif  // PIPELINE_VIEWPORT_STATE_BUILDER_H

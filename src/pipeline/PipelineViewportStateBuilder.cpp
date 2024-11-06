#include "PipelineViewportStateBuilder.h"

PipelineViewportStateBuilder::PipelineViewportStateBuilder(const float width, const float height) {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;
}

PipelineViewportStateBuilder& PipelineViewportStateBuilder::setViewport(const float x, const float y, const float minDepth, const float maxDepth) {
    viewport.x = x;
    viewport.y = y;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return *this;
}

PipelineViewportStateBuilder& PipelineViewportStateBuilder::setScissor(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) {
    scissor.offset = {offsetX, offsetY};
    scissor.extent = {width, height};
    return *this;
}

VkPipelineViewportStateCreateInfo PipelineViewportStateBuilder::build() {
    viewportInfo.pViewports = &viewport;
    viewportInfo.pScissors = &scissor;
    return viewportInfo;
}

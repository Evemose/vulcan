#include "PipelineDepthStencilStateBuilder.h"

PipelineDepthStencilStateBuilder::PipelineDepthStencilStateBuilder() {
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
}

PipelineDepthStencilStateBuilder& PipelineDepthStencilStateBuilder::setDepthTestEnable(VkBool32 enable) {
    depthStencilInfo.depthTestEnable = enable;
    return *this;
}

PipelineDepthStencilStateBuilder& PipelineDepthStencilStateBuilder::setDepthWriteEnable(VkBool32 enable) {
    depthStencilInfo.depthWriteEnable = enable;
    return *this;
}

VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateBuilder::build() const {
    return depthStencilInfo;
}

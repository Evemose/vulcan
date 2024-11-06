#include "PipelineRasterizationStateBuilder.h"

PipelineRasterizationStateBuilder::PipelineRasterizationStateBuilder(VkPolygonMode polygonMode, const float lineWidth) {
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = polygonMode;
    rasterizationInfo.lineWidth = lineWidth;
    rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;
}

PipelineRasterizationStateBuilder& PipelineRasterizationStateBuilder::setCullMode(VkCullModeFlags cullMode) {
    rasterizationInfo.cullMode = cullMode;
    return *this;
}

PipelineRasterizationStateBuilder& PipelineRasterizationStateBuilder::setFrontFace(VkFrontFace frontFace) {
    rasterizationInfo.frontFace = frontFace;
    return *this;
}

PipelineRasterizationStateBuilder& PipelineRasterizationStateBuilder::setDepthBiasEnable(VkBool32 enable) {
    rasterizationInfo.depthBiasEnable = enable;
    return *this;
}

VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateBuilder::build() const {
    return rasterizationInfo;
}

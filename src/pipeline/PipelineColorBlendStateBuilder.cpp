#include "PipelineColorBlendStateBuilder.h"

PipelineColorBlendStateBuilder::PipelineColorBlendStateBuilder() {
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;


    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
}

PipelineColorBlendStateBuilder& PipelineColorBlendStateBuilder::setBlendEnable(VkBool32 enable) {
    colorBlendAttachment.blendEnable = enable;
    return *this;
}

PipelineColorBlendStateBuilder& PipelineColorBlendStateBuilder::setColorWriteMask(VkColorComponentFlags mask) {
    colorBlendAttachment.colorWriteMask = mask;
    return *this;
}

VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateBuilder::build() const {
    return colorBlendInfo;
}

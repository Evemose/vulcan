#include "PipelineBuilder.h"

#include <memory>
#include <stdexcept>

PipelineBuilder::PipelineBuilder(float viewportWidth, float viewportHeight, VkRenderPass renderPass,
                                 VkPipelineLayout pipelineLayout)
    : renderPass(renderPass), pipelineLayout(pipelineLayout) {
    // Configure application-specific viewport state
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = viewportWidth;
    viewport.height = viewportHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = {static_cast<uint32_t>(viewportWidth), static_cast<uint32_t>(viewportHeight)};

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
}

PipelineBuilder &PipelineBuilder::setVertexStage(const VkPipelineShaderStageCreateInfo &vertexStage) {
    this->vertexStage = vertexStage;
    return *this;
}

PipelineBuilder &PipelineBuilder::setFragmentStage(const VkPipelineShaderStageCreateInfo &fragmentStage) {
    this->fragmentStage = fragmentStage;
    return *this;
}

PipelineBuilder &PipelineBuilder::setInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo &inputAssembly) {
    this->inputAssemblyState = inputAssembly;
    return *this;
}

PipelineBuilder &PipelineBuilder::setRasterizationState(const VkPipelineRasterizationStateCreateInfo &rasterization) {
    this->rasterizationState = rasterization;
    return *this;
}

PipelineBuilder &PipelineBuilder::setMultisampleState(const VkPipelineMultisampleStateCreateInfo &multisample) {
    this->multisampleState = multisample;
    return *this;
}

PipelineBuilder &PipelineBuilder::setColorBlendState(const VkPipelineColorBlendStateCreateInfo &colorBlend) {
    this->colorBlendState = colorBlend;
    return *this;
}

PipelineBuilder &PipelineBuilder::setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo &depthStencil) {
    this->depthStencilState = depthStencil;
    return *this;
}

VkPipelineInputAssemblyStateCreateInfo PipelineBuilder::defaultInputAssemblyState() {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return inputAssembly;
}

VkPipelineRasterizationStateCreateInfo PipelineBuilder::defaultRasterizationState() {
    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.lineWidth = 1.0f;
    return rasterization;
}

VkPipelineMultisampleStateCreateInfo PipelineBuilder::defaultMultisampleState() {
    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    return multisample;
}

VkPipelineColorBlendStateCreateInfo PipelineBuilder::defaultColorBlendState() {
    static auto defaultColorBlendAttachment = VkPipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &defaultColorBlendAttachment;
    return colorBlendInfo;
}

VkPipelineDepthStencilStateCreateInfo PipelineBuilder::defaultDepthStencilState() {
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    return depthStencil;
}

VkPipeline PipelineBuilder::build(VkDevice device) const {
    if (!vertexStage.sType || !fragmentStage.sType) {
        throw std::runtime_error("Vertex and fragment shader stages must be set.");
    }

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

    VkPipelineInputAssemblyStateCreateInfo defaultInputAssembly;
    VkPipelineRasterizationStateCreateInfo defaultRasterization;
    VkPipelineMultisampleStateCreateInfo defaultMultisample;
    VkPipelineColorBlendStateCreateInfo defaultColorBlend;
    VkPipelineDepthStencilStateCreateInfo defaultDepthStencil;

    const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyStatePtr = inputAssemblyState ?
            &*inputAssemblyState :
            (defaultInputAssembly = defaultInputAssemblyState(), &defaultInputAssembly);
    const VkPipelineRasterizationStateCreateInfo *rasterizationStatePtr = rasterizationState ?
            &*rasterizationState :
            (defaultRasterization = defaultRasterizationState(), &defaultRasterization);
    const VkPipelineMultisampleStateCreateInfo *multisampleStatePtr = multisampleState ?
            &*multisampleState :
            (defaultMultisample = defaultMultisampleState(), &defaultMultisample);
    const VkPipelineColorBlendStateCreateInfo *colorBlendStatePtr = colorBlendState ?
            &*colorBlendState :
            (defaultColorBlend = defaultColorBlendState(), &defaultColorBlend);
    const VkPipelineDepthStencilStateCreateInfo *depthStencilStatePtr = depthStencilState ?
            &*depthStencilState :
            (defaultDepthStencil = defaultDepthStencilState(), &defaultDepthStencil);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pInputAssemblyState = inputAssemblyStatePtr;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = rasterizationStatePtr;
    pipelineInfo.pMultisampleState = multisampleStatePtr;
    pipelineInfo.pColorBlendState = colorBlendStatePtr;
    pipelineInfo.pDepthStencilState = depthStencilStatePtr;

    pipelineInfo.renderPass = renderPass;
    pipelineInfo.layout = pipelineLayout;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline.");
    }

    return pipeline;
}

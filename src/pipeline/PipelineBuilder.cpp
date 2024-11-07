#include "PipelineBuilder.h"

#include <memory>
#include <stdexcept>

#include "PipelineColorBlendStateBuilder.h"
#include "PipelineDepthStencilStateBuilder.h"
#include "PipelineInputAssemblyStateBuilder.h"
#include "PipelineMultisampleStateBuilder.h"
#include "PipelineRasterizationStateBuilder.h"

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
    PipelineInputAssemblyStateBuilder inputAssemblyBuilder(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    return inputAssemblyBuilder.build();
}

VkPipelineRasterizationStateCreateInfo PipelineBuilder::defaultRasterizationState() {
    PipelineRasterizationStateBuilder rasterizationBuilder(VK_POLYGON_MODE_FILL, 1.0f);
    return rasterizationBuilder.build();
}

VkPipelineMultisampleStateCreateInfo PipelineBuilder::defaultMultisampleState() {
    PipelineMultisampleStateBuilder multisampleBuilder(VK_SAMPLE_COUNT_1_BIT);
    return multisampleBuilder.build();
}

VkPipelineColorBlendStateCreateInfo PipelineBuilder::defaultColorBlendState() {
    PipelineColorBlendStateBuilder colorBlendBuilder;
    return colorBlendBuilder.build();
}

VkPipelineDepthStencilStateCreateInfo PipelineBuilder::defaultDepthStencilState() {
    PipelineDepthStencilStateBuilder depthStencilBuilder;
    return depthStencilBuilder.build();
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

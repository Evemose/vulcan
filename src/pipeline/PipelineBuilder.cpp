#include "PipelineBuilder.h"

#include <memory>
#include <stdexcept>

#include "PipelineColorBlendStateBuilder.h"
#include "PipelineDepthStencilStateBuilder.h"
#include "PipelineInputAssemblyStateBuilder.h"
#include "PipelineLayoutBuilder.h"
#include "PipelineMultisampleStateBuilder.h"
#include "PipelineRasterizationStateBuilder.h"
#include "../swap_chain.h"

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

// VkRenderPass PipelineBuilder::defaultRenderPass() {
//     VkRenderPassCreateInfo renderPassInfo{};
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//     renderPassInfo.attachmentCount = 1;
//
//     VkAttachmentDescription colorAttachment{};
//     colorAttachment.format = swapChainImageFormat;
//     colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//     colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//     renderPassInfo.pAttachments = &colorAttachment;
//     renderPassInfo.subpassCount = 1;
//
//     VkSubpassDescription subpass{};
//     subpass.colorAttachmentCount = 1;
//     VkAttachmentReference colorAttachmentRef{};
//     colorAttachmentRef.attachment = 0;
//     colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//     subpass.pColorAttachments = &colorAttachmentRef;
//     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     renderPassInfo.pSubpasses = &subpass;
//
//     if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create render pass!");
//     }
//
// }

VkPipeline PipelineBuilder::build(Device& device) const {
    if (!vertexStage.sType || !fragmentStage.sType) {
        throw std::runtime_error("Vertex and fragment shader stages must be set.");
    }

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

    VkPipelineInputAssemblyStateCreateInfo defaultInputAssembly;
    VkPipelineRasterizationStateCreateInfo defaultRasterization;
    VkPipelineMultisampleStateCreateInfo defaultMultisample;
    VkPipelineColorBlendStateCreateInfo defaultColorBlend;
    VkPipelineDepthStencilStateCreateInfo defaultDepthStencil;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;

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
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    pipelineInfo.subpass = 0;
    pipelineInfo.renderPass = SwapChain(device, VkExtent2D{
        static_cast<uint32_t>(viewport.width),
        static_cast<uint32_t>(viewport.height)
    }).getRenderPass();
    pipelineInfo.layout = PipelineLayoutBuilder().build(device.device());

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline.");
    }

    vkDestroyShaderModule(device.device(), vertexStage.module, nullptr);
    vkDestroyShaderModule(device.device(), fragmentStage.module, nullptr);

    return pipeline;
}

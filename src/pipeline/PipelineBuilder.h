#ifndef PIPELINE_BUILDER_H
#define PIPELINE_BUILDER_H

#include <vulkan/vulkan.h>
#include <optional>

class Device;

class PipelineBuilder {
public:
    PipelineBuilder(float viewportWidth, float viewportHeight, VkRenderPass renderPass, VkPipelineLayout pipelineLayout);

    PipelineBuilder& setVertexStage(const VkPipelineShaderStageCreateInfo& vertexStage);
    PipelineBuilder& setFragmentStage(const VkPipelineShaderStageCreateInfo& fragmentStage);

    PipelineBuilder& setInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& inputAssembly);
    PipelineBuilder& setRasterizationState(const VkPipelineRasterizationStateCreateInfo& rasterization);
    PipelineBuilder& setMultisampleState(const VkPipelineMultisampleStateCreateInfo& multisample);
    PipelineBuilder& setColorBlendState(const VkPipelineColorBlendStateCreateInfo& colorBlend);
    PipelineBuilder& setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& depthStencil);

    [[nodiscard]] VkPipeline build(Device& device) const;

private:

    VkPipelineShaderStageCreateInfo vertexStage{};
    VkPipelineShaderStageCreateInfo fragmentStage{};
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;

    VkPipelineViewportStateCreateInfo viewportState{};
    VkViewport viewport{};
    VkRect2D scissor{};

    std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssemblyState;
    std::optional<VkPipelineRasterizationStateCreateInfo> rasterizationState;
    std::optional<VkPipelineMultisampleStateCreateInfo> multisampleState;
    std::optional<VkPipelineColorBlendStateCreateInfo> colorBlendState;
    std::optional<VkPipelineDepthStencilStateCreateInfo> depthStencilState;

    [[nodiscard]] static VkPipelineInputAssemblyStateCreateInfo defaultInputAssemblyState();
    [[nodiscard]] static VkPipelineRasterizationStateCreateInfo defaultRasterizationState();
    [[nodiscard]] static VkPipelineMultisampleStateCreateInfo defaultMultisampleState();
    [[nodiscard]] static VkPipelineColorBlendStateCreateInfo defaultColorBlendState();
    [[nodiscard]] static VkPipelineDepthStencilStateCreateInfo defaultDepthStencilState();
    [[nodiscard]] static VkRenderPass defaultRenderPass();
    [[nodiscard]] static VkPipelineLayout defaultPipelineLayout();

};

#endif // PIPELINE_BUILDER_H

// PipelineBuilder.h

#ifndef PIPELINE_BUILDER_H
#define PIPELINE_BUILDER_H

#include <vulkan/vulkan.h>
#include <optional>

class PipelineBuilder {
public:
    PipelineBuilder& setVertexStage(const VkPipelineShaderStageCreateInfo& vertexStage);
    PipelineBuilder& setFragmentStage(const VkPipelineShaderStageCreateInfo& fragmentStage);
    PipelineBuilder& setRenderPass(VkRenderPass rp);
    PipelineBuilder& setPipelineLayout(VkPipelineLayout pl);

    VkPipeline build(VkDevice device) const;

    static void setDefaultRenderPass(VkRenderPass rp);
    static void setDefaultPipelineLayout(VkPipelineLayout pl);

private:
    std::optional<VkPipelineShaderStageCreateInfo> vertexStage;
    std::optional<VkPipelineShaderStageCreateInfo> fragmentStage;
    std::optional<VkRenderPass> renderPass;
    std::optional<VkPipelineLayout> pipelineLayout;

    static inline VkRenderPass defaultRenderPass = VK_NULL_HANDLE;
    static inline VkPipelineLayout defaultPipelineLayout = VK_NULL_HANDLE;
};

#endif // PIPELINE_BUILDER_H

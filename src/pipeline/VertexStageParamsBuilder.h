#ifndef VERTEX_STAGE_BUILDER_H
#define VERTEX_STAGE_BUILDER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

class VertexStageParamsBuilder {
public:
    VertexStageParamsBuilder& setShaderModule(VkShaderModule module);
    VertexStageParamsBuilder& setEntryPoint(const char* entryPoint);
    VertexStageParamsBuilder& setFlags(VkPipelineShaderStageCreateFlags stageFlags);
    VertexStageParamsBuilder& setSpecializationInfo(const VkSpecializationInfo& specInfo);
    VertexStageParamsBuilder& setPushConstants(const std::vector<uint8_t>& pushData);

    [[nodiscard]] VkPipelineShaderStageCreateInfo build() const;

    static void setDefaultShaderModule(VkShaderModule module);

private:
    std::optional<VkShaderModule> shaderModule;
    std::optional<const char*> entryPointName;
    std::optional<VkPipelineShaderStageCreateFlags> flags;
    std::optional<VkSpecializationInfo> specializationInfo;
    std::optional<std::vector<uint8_t>> pushConstants;

    static inline VkShaderModule defaultShaderModule = VK_NULL_HANDLE;
};

#endif

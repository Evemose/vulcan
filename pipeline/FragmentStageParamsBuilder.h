#ifndef FRAGMENTSTAGEBUILDER_H
#define FRAGMENTSTAGEBUILDER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

class FragmentStageParamsBuilder {
public:
    FragmentStageParamsBuilder& setShaderModule(VkShaderModule module);
    FragmentStageParamsBuilder& setEntryPoint(const char* entryPoint);
    FragmentStageParamsBuilder& setFlags(VkPipelineShaderStageCreateFlags stageFlags);
    FragmentStageParamsBuilder& setSpecializationInfo(const VkSpecializationInfo& specInfo);
    FragmentStageParamsBuilder& setPushConstants(const std::vector<uint8_t>& pushData);

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



#endif //FRAGMENTSTAGEBUILDER_H

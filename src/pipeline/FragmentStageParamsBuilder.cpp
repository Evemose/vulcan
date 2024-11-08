#include "FragmentStageParamsBuilder.h"

FragmentStageParamsBuilder& FragmentStageParamsBuilder::setShaderModule(VkShaderModule module) {
    shaderModule = module;
    return *this;
}

FragmentStageParamsBuilder& FragmentStageParamsBuilder::setEntryPoint(const char* entryPoint) {
    entryPointName = entryPoint;
    return *this;
}

FragmentStageParamsBuilder& FragmentStageParamsBuilder::setFlags(VkPipelineShaderStageCreateFlags stageFlags) {
    flags = stageFlags;
    return *this;
}

FragmentStageParamsBuilder& FragmentStageParamsBuilder::setSpecializationInfo(const VkSpecializationInfo& specInfo) {
    specializationInfo = specInfo;
    return *this;
}

FragmentStageParamsBuilder& FragmentStageParamsBuilder::setPushConstants(const std::vector<uint8_t>& pushData) {
    pushConstants = pushData;
    return *this;
}

VkPipelineShaderStageCreateInfo FragmentStageParamsBuilder::build() const {
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stageInfo.module = shaderModule.value_or(defaultShaderModule);
    stageInfo.pName = entryPointName.value_or("main");
    stageInfo.flags = flags.value_or(0);
    stageInfo.pSpecializationInfo = specializationInfo.has_value() ? &specializationInfo.value() : nullptr;
    stageInfo.pNext = nullptr;

    return stageInfo;
}

void FragmentStageParamsBuilder::setDefaultShaderModule(VkShaderModule module) {
    defaultShaderModule = module;
}

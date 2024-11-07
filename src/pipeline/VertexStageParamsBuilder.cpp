#include "VertexStageParamsBuilder.h"

VertexStageParamsBuilder& VertexStageParamsBuilder::setShaderModule(VkShaderModule module) {
    shaderModule = module;
    return *this;
}

VertexStageParamsBuilder& VertexStageParamsBuilder::setEntryPoint(const char* entryPoint) {
    entryPointName = entryPoint;
    return *this;
}

VertexStageParamsBuilder& VertexStageParamsBuilder::setFlags(VkPipelineShaderStageCreateFlags stageFlags) {
    flags = stageFlags;
    return *this;
}

VertexStageParamsBuilder& VertexStageParamsBuilder::setSpecializationInfo(const VkSpecializationInfo& specInfo) {
    specializationInfo = specInfo;
    return *this;
}

VertexStageParamsBuilder& VertexStageParamsBuilder::setPushConstants(const std::vector<uint8_t>& pushData) {
    pushConstants = pushData;
    return *this;
}

VkPipelineShaderStageCreateInfo VertexStageParamsBuilder::build() const {
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stageInfo.module = shaderModule.value();
    stageInfo.pName = entryPointName.value_or("main");
    stageInfo.flags = flags.value_or(0);
    stageInfo.pSpecializationInfo = specializationInfo.has_value() ? &specializationInfo.value() : nullptr;

    return stageInfo;
}
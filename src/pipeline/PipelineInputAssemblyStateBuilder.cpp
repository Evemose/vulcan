#include "PipelineInputAssemblyStateBuilder.h"

PipelineInputAssemblyStateBuilder::PipelineInputAssemblyStateBuilder(VkPrimitiveTopology topology) {
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = topology;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;  // Default value
}

PipelineInputAssemblyStateBuilder& PipelineInputAssemblyStateBuilder::setPrimitiveRestartEnable(VkBool32 enable) {
    inputAssemblyInfo.primitiveRestartEnable = enable;
    return *this;
}

VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateBuilder::build() const {
    return inputAssemblyInfo;
}

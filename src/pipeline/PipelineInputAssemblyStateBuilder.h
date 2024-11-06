#ifndef PIPELINE_INPUT_ASSEMBLY_STATE_BUILDER_H
#define PIPELINE_INPUT_ASSEMBLY_STATE_BUILDER_H

#include <vulkan/vulkan.h>

class PipelineInputAssemblyStateBuilder {
public:
    explicit PipelineInputAssemblyStateBuilder(VkPrimitiveTopology topology);

    PipelineInputAssemblyStateBuilder& setPrimitiveRestartEnable(VkBool32 enable);

    [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo build() const;

private:
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
};

#endif  // PIPELINE_INPUT_ASSEMBLY_STATE_BUILDER_H

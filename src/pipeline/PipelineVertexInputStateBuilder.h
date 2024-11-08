#ifndef PIPELINEVERTEXINPUTSTATEBUILDER_H
#define PIPELINEVERTEXINPUTSTATEBUILDER_H
#include <vector>

#include "PipelineInputAssemblyStateBuilder.h"


class PipelineVertexInputStateBuilder {
private:
    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

public:
    PipelineVertexInputStateBuilder& addBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);
    PipelineVertexInputStateBuilder& addAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

    [[nodiscard]] VkPipelineVertexInputStateCreateInfo build() const;
};



#endif //PIPELINEVERTEXINPUTSTATEBUILDER_H

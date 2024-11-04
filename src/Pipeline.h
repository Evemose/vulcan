#ifndef PIPELINE_H
#define PIPELINE_H
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class Device;

class Pipeline {
private:
    Device& device;
    VkPipeline pipeline;
    VkShaderModule vertexShaderModule;
public:
    Pipeline(Device& device, VkPipeline pipeline, VkShaderModule vertexShaderModule);
    ~Pipeline();
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
};



#endif //PIPELINE_H

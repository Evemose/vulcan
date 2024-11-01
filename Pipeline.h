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
    class PipelineBuilder {
    private:
        Device& deviceField;
        std::string const vertexShaderFile;
        std::string const fragmentShaderFile;
        PipelineBuilder(Device& device, std::string vertexShaderFile, std::string fragmentShaderFile);
        void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);
        friend class Pipeline;
    public:
        Pipeline build();
    };
    Pipeline(Device& device, VkPipeline pipeline, VkShaderModule vertexShaderModule);
    ~Pipeline();
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    static PipelineBuilder builder(Device& device, std::string vertexShaderFile, std::string fragmentShaderFile);
};



#endif //PIPELINE_H

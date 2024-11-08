#include "device.h"
#include "files.h"
#include "shaders.h"
#include "Window.h"
#include "pipeline/FragmentStageParamsBuilder.h"
#include "pipeline/PipelineBuilder.h"
#include "pipeline/VertexStageParamsBuilder.h"

int main() {
    auto window = Window(800, 600, "Vulkan");
    auto device = Device(window);
    auto vkDevice = device.device();

    PipelineBuilder builder(800, 600, nullptr, nullptr);
    VkShaderModule vertexShaderModule;
    createShaderModule(vkDevice, myutils::readFile("../build/shader.vert.spv"), vertexShaderModule);
    builder.setVertexStage(VertexStageParamsBuilder()
        .setShaderModule(vertexShaderModule)
        .build());

    VkShaderModule fragmentShaderModule;
    createShaderModule(vkDevice, myutils::readFile("../build/shader.frag.spv"), fragmentShaderModule);
    builder.setFragmentStage(FragmentStageParamsBuilder()
        .setShaderModule(fragmentShaderModule)
        .build());

    builder.build(device);
    while (!window.shouldClose()) {
        glfwPollEvents();
    }
}

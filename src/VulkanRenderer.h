#ifndef VULKARENDERER_H
#define VULKARENDERER_H

#define GLFW_INCLUDE_VULKAN
#include <memory>
#include <tuple>
#include <tuple>
#include <GLFW/glfw3.h>
#include <vector>

#include "Mesh.h"
#include "MeshFactory.h"
#include "utils.h"

class VulkanRenderer {
public:
    static VulkanRenderer create(GLFWwindow *window);

    void drawFrame(const std::vector<Mesh> &meshes);

    [[nodiscard]] MeshFactory getMeshFactory() const;

    ~VulkanRenderer();

private:
    typedef struct MVP {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;
    } MVP;

    typedef struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } Device;

    typedef struct {
        int graphicsFamily = -1;
        int presentFamily = -1;

        [[nodiscard]] bool isValid() const {
            return graphicsFamily >= 0
                   && presentFamily >= 0;
        }
    } QueueFamilyIndices;

    typedef struct {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        [[nodiscard]] bool isValid() const {
            return !formats.empty() && !presentModes.empty();
        }
    } SwapChainDetails;

    typedef struct {
        VkImage image;
        VkImageView imageView;
    } SwapChainImage;

    typedef struct {
        const VkSwapchainKHR swapChain;
        const VkFormat swapChainImageFormat;
        const VkExtent2D swapChainExtent;
        const std::vector<SwapChainImage> swapChainImages;
    } SwapChainAndMetadata;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    const GLFWwindow *window;
    const VkInstance instance;
    const Device device{};
    const QueueFamilyIndices queues;
    const VkSurfaceKHR surface;
    const VkQueue graphicsQueue;
    const VkQueue presentQueue;
    const SwapChainAndMetadata swapChainAndMetadata;
    const VkRenderPass renderPass;
    const VkPipeline graphicsPipeline;
    const VkPipelineLayout pipelineLayout;
    const std::vector<VkFramebuffer> swapChainFramebuffers;
    const VkCommandPool commandPool;
    const std::vector<VkCommandBuffer> commandBuffers;
    const std::vector<VkSemaphore> imageAvailableSemaphores;
    const std::vector<VkSemaphore> renderFinishedSemaphores;
    const std::vector<VkFence> inFlightFences;
    const MeshFactory meshFactory;
    const VkDescriptorSetLayout descriptorSetLayout;
    const std::vector<VkBuffer> uniformBuffers;
    const std::vector<VkDeviceMemory> uniformBuffersMemory;
    const VkDescriptorPool descriptorPool;
    const std::vector<VkDescriptorSet> descriptorSets;

    int currentFrame = 0;

    VulkanRenderer(
        const GLFWwindow *window, VkInstance instance, Device device, QueueFamilyIndices queues,
        VkSurfaceKHR surface, SwapChainAndMetadata swapChainAndMetadata, VkRenderPass renderPass,
        VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
        std::vector<VkFramebuffer> swapChainFramebuffers,
        VkCommandPool graphicsPool, std::vector<VkCommandBuffer> commandBuffers,
        std::vector<VkSemaphore> imageAvailableSemaphore,
        std::vector<VkSemaphore> renderFinishedSemaphore,
        std::vector<VkFence> inFlightFences, VkDescriptorSetLayout descriptorSetLayout,
        std::vector<VkBuffer> uniformBuffers, std::vector<VkDeviceMemory> uniformBuffersMemory,
        VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet> descriptorSets
    );

    static VkPhysicalDevice getPhysicalDevice(VkInstance instance, VkSurfaceKHR surfaceTuSupport);

    static std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfos(QueueFamilyIndices queues);

    static VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices queues);

    static std::tuple<Device, QueueFamilyIndices, SwapChainDetails> getDeviceAndDetails(
        VkInstance instance, VkSurfaceKHR surfaceToSupport);

    static SwapChainDetails getSwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

    static QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surfaceTuSupport);

    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surfaceTuSupport);

    static VkImageView createImageView(VkDevice vkDevice, VkImage image, VkFormat format,
                                       VkImageAspectFlagBits vkImageAspectFlagBits);

    static SwapChainAndMetadata createSwapChain(Device device, VkSurfaceKHR surface, GLFWwindow *window);

    static VkRenderPass createRenderPass(VkDevice device, const SwapChainAndMetadata &swapChainAndMetadata);

    static std::tuple<VkPipeline, VkPipelineLayout> createGraphicsPipeline(
        VkDevice device, const SwapChainAndMetadata &swapChainAndMetadata, VkRenderPass renderPass,
        VkDescriptorSetLayout
        descriptorSetLayout);

    static std::vector<VkFramebuffer> createFramebuffers(VkDevice device,
                                                         const SwapChainAndMetadata &swapChainAndMetadata,
                                                         VkRenderPass renderPass);

    static std::tuple<std::vector<VkSemaphore>, std::vector<VkSemaphore>, std::vector<VkFence> > createSync(
        VkDevice device);

    static std::tuple<std::vector<VkBuffer>, std::vector<VkDeviceMemory> > createUniformBuffers(
        Device device, size_t buffersCount);

    static std::vector<VkDescriptorSet> createDescriptorSets(
        VkDevice vkDevice, VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        const std::vector<VkBuffer> &buffers, size_t setsCount);

    void recordCommand(uint32_t imageIndex, const std::vector<Mesh> &meshes, const MVP& mvp) const;

    void submitCommand(uint32_t imageIndex);
};


#endif //VULKARENDERER_H

#include "VulkanRenderer.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <ranges>
#include <set>

#include <stdexcept>
#include <tuple>
#include <utility>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "swapchain_choices.h"

const auto requiredExtensions = std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    return surface;
}

std::vector<const char *> getExtensions() {
    auto extensions = std::vector<const char *>();
    uint32_t glfwExtensionsCount;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    for (auto i = 0u; i < glfwExtensionsCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

VkApplicationInfo getAppInfo() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test Window";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    return appInfo;
}

VkInstance createInstance() {
    const VkApplicationInfo appInfo = getAppInfo();

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    const char *layers[] = {
        "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_profiles"
    };
    createInfo.enabledLayerCount = 2;
    createInfo.ppEnabledLayerNames = layers;

    VkInstance instance;
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));

    return instance;
}

VkCommandPool createCommandPool(VkDevice vkDevice, int familyIdx) {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = familyIdx;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool commandPool;
    VK_CHECK(vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &commandPool));
    return commandPool;
}

std::vector<VkCommandBuffer> createCommandBuffers(
    VkDevice vkDevice, VkCommandPool vkCommandPool,
    const std::vector<VkFramebuffer> &vector
) {
    std::vector<VkCommandBuffer> commandBuffers(vector.size());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(vkDevice, &allocInfo, commandBuffers.data()));

    return commandBuffers;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice vkDevice) {
    VkDescriptorSetLayoutBinding mvpLayoutBinding = {};
    mvpLayoutBinding.binding = 0;
    mvpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mvpLayoutBinding.descriptorCount = 1;
    mvpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    mvpLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &mvpLayoutBinding;

    VkDescriptorSetLayout descriptorSetLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &descriptorSetLayout));

    return descriptorSetLayout;
}

std::tuple<std::vector<VkBuffer>, std::vector<VkDeviceMemory> > VulkanRenderer::createUniformBuffers(
    Device device, size_t buffersCount
) {
    std::vector<VkBuffer> uniformBuffers(buffersCount);
    std::vector<VkDeviceMemory> uniformBuffersMemory(buffersCount);

    for (auto i = 0; i < buffersCount; i++) {
        auto bufferSize = sizeof(MVP);
        auto [buffer, memory] = createBuffer(
            device.logicalDevice, device.physicalDevice,
            bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        uniformBuffers[i] = buffer;
        uniformBuffersMemory[i] = memory;
    }

    return {uniformBuffers, uniformBuffersMemory};
}

VkDescriptorPool createDescriptorPool(VkDevice vkDevice, size_t size) {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(size);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(size);

    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &descriptorPool));

    return descriptorPool;
}

std::vector<VkDescriptorSet> VulkanRenderer::createDescriptorSets(
    VkDevice vkDevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
    const std::vector<VkBuffer> &buffers, size_t setsCount
) {
    std::vector<VkDescriptorSet> sets(setsCount);
    std::vector layoutsVector(setsCount, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(setsCount);
    allocInfo.pSetLayouts = layoutsVector.data();

    VK_CHECK(vkAllocateDescriptorSets(vkDevice, &allocInfo, sets.data()));

    for (auto i = 0u; i < setsCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(MVP);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(vkDevice, 1, &descriptorWrite, 0, nullptr);
    }

    return sets;
}

VulkanRenderer VulkanRenderer::create(GLFWwindow *window) {
    auto instance = createInstance();
    auto surface = createSurface(instance, window);
    auto [device, queues, swapChainDetails] = getDeviceAndDetails(instance, surface);
    auto swapChainAndMeta = createSwapChain(device, surface, window);
    auto renderPass = createRenderPass(device.logicalDevice, swapChainAndMeta);
    auto descriptorSetLayout = createDescriptorSetLayout(device.logicalDevice);
    auto [graphicsPipeline, layout] = createGraphicsPipeline(device.logicalDevice, swapChainAndMeta, renderPass,
                                                             descriptorSetLayout);
    auto swapChainFramebuffers = createFramebuffers(device.logicalDevice, swapChainAndMeta, renderPass);
    auto graphicsCommandPool = createCommandPool(device.logicalDevice, queues.graphicsFamily);
    auto commandBuffers = createCommandBuffers(
        device.logicalDevice, graphicsCommandPool, swapChainFramebuffers
    );
    auto [imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences]
            = createSync(device.logicalDevice);
    auto [uniformBuffers, uniformBuffersMemory] = createUniformBuffers(
        device,
        swapChainAndMeta.swapChainImages.size()
    );
    auto descriptorPool = createDescriptorPool(device.logicalDevice, swapChainAndMeta.swapChainImages.size());
    auto descriptorSets = createDescriptorSets(
        device.logicalDevice, descriptorPool, descriptorSetLayout, uniformBuffers,
        swapChainAndMeta.swapChainImages.size()
    );
    return {
        window, instance, device, queues, surface, swapChainAndMeta, renderPass, layout, graphicsPipeline,
        swapChainFramebuffers, graphicsCommandPool, commandBuffers, imageAvailableSemaphores, renderFinishedSemaphores,
        inFlightFences, descriptorSetLayout, uniformBuffers, uniformBuffersMemory, descriptorPool, descriptorSets
    };
}

void VulkanRenderer::recordCommand(uint32_t imageIndex, const std::vector<Mesh> &meshes, const MVP &mvp) const {

    void* data;
    vkMapMemory(device.logicalDevice, uniformBuffersMemory[imageIndex], 0, sizeof(MVP), 0, &data);
    memcpy(data, &mvp, sizeof(MVP));
    vkUnmapMemory(device.logicalDevice, uniformBuffersMemory[imageIndex]);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainAndMetadata.swapChainExtent;

    VkClearValue clearValues[] = {
        {0.6f, 0.65f, 0.6f, 1.0f}
    };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearValues;

    VK_CHECK(vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo));
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    VkDeviceSize offsets[] = {0};
    for (const auto &mesh: meshes) {
        VkBuffer vertexBuffers[] = {mesh.getVertexBuffer()};
        vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[imageIndex], mesh.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
            &descriptorSets[imageIndex], 0, nullptr
        );
        vkCmdDrawIndexed(commandBuffers[imageIndex], mesh.getIndexCount(), 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    VK_CHECK(vkEndCommandBuffer(commandBuffers[imageIndex]));
}

void VulkanRenderer::submitCommand(uint32_t imageIndex) {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChainAndMetadata.swapChain;
    presentInfo.pImageIndices = &imageIndex;

    VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::drawFrame(const std::vector<Mesh> &meshes) {
    vkWaitForFences(device.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device.logicalDevice, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device.logicalDevice, swapChainAndMetadata.swapChain, UINT64_MAX,
                          imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffers[imageIndex], 0);

    MVP mvp = {};
    mvp.projection = glm::perspective(
        glm::radians(45.0f),
        swapChainAndMetadata.swapChainExtent.width / static_cast<float>(swapChainAndMetadata.swapChainExtent.height),
        0.1f, 10.0f
    );
    mvp.view = lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    mvp.model = glm::mat4(1.0f);

    mvp.projection[1][1] *= -1;

    recordCommand(imageIndex, meshes, mvp);
    submitCommand(imageIndex);
}

MeshFactory VulkanRenderer::getMeshFactory() const {
    return meshFactory;
}

VkQueue getQueue(const VkDevice device, const int queueIdx) {
    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, queueIdx, 0, &graphicsQueue);
    return graphicsQueue;
}

VkQueue getPresentQueue(const VkDevice device, const int presentFamilyIdx) {
    VkQueue presentQueue;
    vkGetDeviceQueue(device, presentFamilyIdx, 0, &presentQueue);
    return presentQueue;
}

VulkanRenderer::VulkanRenderer(
    const GLFWwindow *window, VkInstance instance, Device device, QueueFamilyIndices queues,
    VkSurfaceKHR surface, SwapChainAndMetadata swapChainAndMetadata,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout, VkPipeline graphicsPipeline,
    std::vector<VkFramebuffer> swapChainFramebuffers, VkCommandPool commandPool,
    std::vector<VkCommandBuffer> commandBuffers, std::vector<VkSemaphore> imageAvailableSemaphore,
    std::vector<VkSemaphore> renderFinishedSemaphore, std::vector<VkFence> inFlightFences,
    VkDescriptorSetLayout descriptorSetLayout,
    std::vector<VkBuffer> uniformBuffers, std::vector<VkDeviceMemory> uniformBuffersMemory,
    VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet> descriptorSets
): window(window), instance(instance), device(device), queues(queues), surface(surface),
   graphicsQueue(getQueue(device.logicalDevice, queues.graphicsFamily)),
   presentQueue(getPresentQueue(device.logicalDevice, queues.presentFamily)),
   swapChainAndMetadata(std::move(swapChainAndMetadata)), renderPass(renderPass), graphicsPipeline(graphicsPipeline),
   pipelineLayout(pipelineLayout), swapChainFramebuffers(std::move(swapChainFramebuffers)), commandPool(commandPool),
   commandBuffers(std::move(commandBuffers)), imageAvailableSemaphores(std::move(imageAvailableSemaphore)),
   renderFinishedSemaphores(std::move(renderFinishedSemaphore)), inFlightFences(std::move(inFlightFences)),
   meshFactory({
       device.physicalDevice, device.logicalDevice, commandPool, getQueue(device.logicalDevice, queues.graphicsFamily)
   }),
   descriptorSetLayout(descriptorSetLayout), uniformBuffers(std::move(uniformBuffers)),
   uniformBuffersMemory(std::move(uniformBuffersMemory)), descriptorPool(descriptorPool),
   descriptorSets(std::move(descriptorSets)) {
}

VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(device.logicalDevice);
    vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr);
    for (auto i = 0u; i < swapChainAndMetadata.swapChainImages.size(); i++) {
        vkDestroyBuffer(device.logicalDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(device.logicalDevice, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device.logicalDevice, descriptorPool, nullptr);
    for (const auto &fence: inFlightFences) {
        vkDestroyFence(device.logicalDevice, fence, nullptr);
    }
    for (const auto &semaphore: imageAvailableSemaphores) {
        vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
    }
    for (const auto &semaphore: renderFinishedSemaphores) {
        vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
    }
    vkDestroyCommandPool(device.logicalDevice, commandPool, nullptr);
    for (const auto &framebuffer: swapChainFramebuffers) {
        vkDestroyFramebuffer(device.logicalDevice, framebuffer, nullptr);
    }
    vkDestroyPipeline(device.logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device.logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(device.logicalDevice, renderPass, nullptr);
    for (const auto &swapChainImage: swapChainAndMetadata.swapChainImages) {
        vkDestroyImageView(device.logicalDevice, swapChainImage.imageView, nullptr);
    }
    vkDestroySwapchainKHR(device.logicalDevice, swapChainAndMetadata.swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

VkPhysicalDevice VulkanRenderer::getPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surfaceTuSupport) {
    std::vector<VkPhysicalDevice> devices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan devices found");
    }
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device: devices) {
        if (isDeviceSuitable(device, surfaceTuSupport)) {
            return device;
        }
    }

    throw std::runtime_error("No suitable Vulkan devices found");
}

std::vector<VkDeviceQueueCreateInfo> VulkanRenderer::getQueueCreateInfos(const QueueFamilyIndices queues) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (std::set uniqueQueueFamilies = {queues.graphicsFamily, queues.presentFamily}; auto idx: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = idx;
        queueCreateInfo.queueCount = 1;
        constexpr float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    return queueCreateInfos;
}

std::vector<VkExtensionProperties> getAvailableExtensions(const VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    auto availableExtensions = std::vector<VkExtensionProperties>(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    return availableExtensions;
}

VkDevice VulkanRenderer::createLogicalDevice(const VkPhysicalDevice physicalDevice, const QueueFamilyIndices queues) {
    const auto queueCreateInfos = getQueueCreateInfos(queues);
    const auto availableExtensions = getAvailableExtensions(physicalDevice);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    std::vector<const char *> enabledExtensions{};
    enabledExtensions.reserve(requiredExtensions.size());
    for (const auto &extension: requiredExtensions) {
        enabledExtensions.push_back(extension);
    }

    if (std::ranges::find_if(availableExtensions, [](const auto &extension) {
        return strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0;
    }) != availableExtensions.end()) {
        enabledExtensions.push_back("VK_KHR_portability_subset");
    }

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice logicalDevice;
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice));

    return logicalDevice;
}

std::tuple<VulkanRenderer::Device, VulkanRenderer::QueueFamilyIndices, VulkanRenderer::SwapChainDetails>
VulkanRenderer::getDeviceAndDetails(
    const VkInstance instance, const VkSurfaceKHR surfaceToSupport
) {
    const auto physicalDevice = getPhysicalDevice(instance, surfaceToSupport);
    const auto queues = getQueueFamilies(physicalDevice, surfaceToSupport);
    const auto swapChainDetails = getSwapChainDetails(physicalDevice, surfaceToSupport);
    const auto logicalDevice = createLogicalDevice(physicalDevice, queues);
    const Device device = {physicalDevice, logicalDevice};
    return {device, queues, swapChainDetails};
}

VulkanRenderer::SwapChainDetails VulkanRenderer::getSwapChainDetails(
    const VkPhysicalDevice device, const VkSurfaceKHR surface
) {
    SwapChainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);
    if (details.surfaceCapabilities.maxImageCount == 0) {
        details.surfaceCapabilities.maxImageCount = UINT32_MAX;
    }

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::getQueueFamilies(
    const VkPhysicalDevice device, const VkSurfaceKHR surfaceTuSupport
) {
    QueueFamilyIndices indices;
    auto queueFamilyCount = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    auto queueFamilies = std::vector<VkQueueFamilyProperties>(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (auto i = 0; i < queueFamilyCount && !indices.isValid(); i++) {
        if (const auto queueFamily = queueFamilies[i];
            queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            VkBool32 presentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surfaceTuSupport, &presentationSupport);
            if (presentationSupport) {
                indices.presentFamily = i;
            }
        }
    }

    return indices;
}

bool hasAllRequiredExtensions(const std::vector<VkExtensionProperties> &availableExtensions) {
    for (const auto &requiredExtension: requiredExtensions) {
        auto found = false;
        for (const auto &availableExtension: availableExtensions) {
            if (strcmp(availableExtension.extensionName, requiredExtension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

bool VulkanRenderer::isDeviceSuitable(const VkPhysicalDevice device, const VkSurfaceKHR surfaceTuSupport) {
    return getQueueFamilies(device, surfaceTuSupport).isValid()
           && hasAllRequiredExtensions(getAvailableExtensions(device))
           && getSwapChainDetails(device, surfaceTuSupport).isValid();
}

VkImageView VulkanRenderer::createImageView(
    VkDevice vkDevice, VkImage image, VkFormat format, VkImageAspectFlagBits vkImageAspectFlagBits
) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask = vkImageAspectFlagBits;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK(vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView));
    return imageView;
}

VulkanRenderer::SwapChainAndMetadata VulkanRenderer::createSwapChain(
    const Device device, const VkSurfaceKHR surface, GLFWwindow *window
) {
    const auto swapChainDetails = getSwapChainDetails(device.physicalDevice, surface);

    const auto bestSurfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
    const auto bestPresentMode = chooseSwapPresentMode(swapChainDetails.presentModes);
    const auto extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities, window);

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.imageFormat = bestSurfaceFormat.format;
    createInfo.imageColorSpace = bestSurfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.presentMode = bestPresentMode;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    const auto minSwapChainImages = std::min(
        swapChainDetails.surfaceCapabilities.minImageCount + 1,
        swapChainDetails.surfaceCapabilities.maxImageCount
    );
    createInfo.minImageCount = minSwapChainImages;

    auto queues = getQueueFamilies(device.physicalDevice, surface);

    if (queues.graphicsFamily != queues.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        const uint32_t queueFamilyIndices[] = {
            static_cast<uint32_t>(queues.graphicsFamily),
            static_cast<uint32_t>(queues.presentFamily)
        };
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain;
    VK_CHECK(vkCreateSwapchainKHR(device.logicalDevice, &createInfo, nullptr, &swapChain));

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, nullptr);
    auto swapChainVkImages = std::vector<VkImage>(imageCount);
    vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, swapChainVkImages.data());

    auto mappedImages = std::vector<SwapChainImage>(imageCount);
    for (auto i = 0u; i < imageCount; i++) {
        auto &image = mappedImages[i];
        image.image = swapChainVkImages[i];
        image.imageView = createImageView(device.logicalDevice, image.image, bestSurfaceFormat.format,
                                          VK_IMAGE_ASPECT_COLOR_BIT);
    }

    return {swapChain, bestSurfaceFormat.format, extent, mappedImages};
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

VkRenderPass VulkanRenderer::createRenderPass(VkDevice device, const SwapChainAndMetadata &swapChainAndMetadata) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChainAndMetadata.swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    std::array<VkSubpassDependency, 2> dependencies{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstSubpass = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VkRenderPass renderPass;
    VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

std::tuple<VkPipeline, VkPipelineLayout> VulkanRenderer::createGraphicsPipeline(
    VkDevice device, const SwapChainAndMetadata &swapChainAndMetadata,
    VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout) {
    auto vertexCode = readGlslResource("shader.vert");
    auto fragmentCode = readGlslResource("shader.frag");

    auto vertexModule = createShaderModule(device, vertexCode);
    auto fragmentModule = createShaderModule(device, fragmentCode);

    VkPipelineShaderStageCreateInfo vertexStageInfo = {};
    vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageInfo.module = vertexModule;
    vertexStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageInfo = {};
    fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageInfo.module = fragmentModule;
    fragmentStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStageInfo, fragmentStageInfo};

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainAndMetadata.swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainAndMetadata.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainAndMetadata.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                          | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipeline graphicsPipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));

    vkDestroyShaderModule(device, vertexModule, nullptr);
    vkDestroyShaderModule(device, fragmentModule, nullptr);

    return {graphicsPipeline, pipelineLayout};
}

std::vector<VkFramebuffer> VulkanRenderer::createFramebuffers(VkDevice device,
                                                              const SwapChainAndMetadata &swapChainAndMetadata,
                                                              VkRenderPass renderPass) {
    std::vector<VkFramebuffer> swapChainFramebuffers(swapChainAndMetadata.swapChainImages.size());
    for (size_t i = 0; i < swapChainAndMetadata.swapChainImages.size(); i++) {
        VkImageView attachments[] = {
            swapChainAndMetadata.swapChainImages[i].imageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainAndMetadata.swapChainExtent.width;
        framebufferInfo.height = swapChainAndMetadata.swapChainExtent.height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]));
    }
    return swapChainFramebuffers;
}

std::tuple<std::vector<VkSemaphore>, std::vector<VkSemaphore>, std::vector<VkFence> >
VulkanRenderer::createSync(VkDevice device) {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    std::vector<VkSemaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkSemaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkFence> inFlightFences(MAX_FRAMES_IN_FLIGHT);

    for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
    }

    return {imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences};
}

#include "VulkanRendererConstructor.h"

#include <ranges>
#include <set>
#include <unordered_set>

#include "utils.h"
#include "../io/files.h"
#include "../ViewProjection.h"

std::unordered_set preferableFormats = {
    vk::Format::eB8G8R8Unorm,
    vk::Format::eR8G8B8Unorm
};

auto preferableColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

namespace enjine {
    const auto requiredExtensions = std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    struct SwapChainDetails {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueHandles {
        QueueHandle graphicsQueue = QueueHandle{-1, nullptr};
        QueueHandle presentQueue = QueueHandle{-1, nullptr};

        [[nodiscard]] bool areIndicesValid() const {
            return graphicsQueue.index >= 0 && presentQueue.index >= 0;
        }
    };

    vk::UniqueInstance createInstance();

    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice &device, vk::SurfaceKHR surface);

    QueueHandles getRawQueueHandles(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    void VulkanRendererConstructor::initQueues() {
        auto rawQueueHandles = getRawQueueHandles(physicalDevice, surface.get());
        graphicsQueue = QueueHandle{
            rawQueueHandles.graphicsQueue.index,
            logicalDevice->getQueue(rawQueueHandles.graphicsQueue.index, 0)
        };
        presentQueue = QueueHandle{
            rawQueueHandles.presentQueue.index,
            logicalDevice->getQueue(rawQueueHandles.presentQueue.index, 0)
        };
    }

    void VulkanRendererConstructor::createRenderPass() {
        vk::AttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainHandle.swapChainImageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        std::array<vk::SubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
        dependencies[0].dstSubpass = 0;
        dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
                                        vk::AccessFlagBits::eColorAttachmentWrite;

        dependencies[1].srcSubpass = 0;
        dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
                                        vk::AccessFlagBits::eColorAttachmentWrite;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;

        vk::RenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        renderPass = logicalDevice->createRenderPassUnique(renderPassInfo);
    }

    vk::UniqueShaderModule VulkanRendererConstructor::createShaderModule(const std::vector<char> &code) {
        vk::ShaderModuleCreateInfo shaderModuleInfo = {};
        shaderModuleInfo.codeSize = code.size();
        shaderModuleInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        shaderModuleInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        shaderModuleInfo.pNext = nullptr;

        return logicalDevice->createShaderModuleUnique(shaderModuleInfo);
    }

    void VulkanRendererConstructor::createGraphicsPipeline() {
        auto vertexCode = readSpvResource("shader.vert");
        auto fragmentCode = readSpvResource("shader.frag");

        auto vertexModule = createShaderModule(vertexCode);
        auto fragmentModule = createShaderModule(fragmentCode);

        vk::PipelineShaderStageCreateInfo vertexStageInfo = {};
        vertexStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertexStageInfo.module = vertexModule.get();
        vertexStageInfo.pName = "main";
        vertexStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;

        vk::PipelineShaderStageCreateInfo fragmentStageInfo = {};
        fragmentStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragmentStageInfo.module = fragmentModule.get();
        fragmentStageInfo.pName = "main";
        fragmentStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;

        vk::VertexInputAttributeDescription positionDescription = {};
        positionDescription.binding = 0;
        positionDescription.location = 0;
        positionDescription.format = vk::Format::eR32G32B32Sfloat;
        positionDescription.offset = offsetof(Vertex, pos);

        vk::VertexInputAttributeDescription colorDescription = {};
        colorDescription.binding = 0;
        colorDescription.location = 1;
        colorDescription.format = vk::Format::eR32G32B32Sfloat;
        colorDescription.offset = offsetof(Vertex, color);

        std::array attributesDescription = {positionDescription, colorDescription};

        vk::VertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = attributesDescription.size();
        vertexInputInfo.pVertexAttributeDescriptions = attributesDescription.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
        inputAssemblyInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssemblyInfo.pNext = nullptr;
        inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainHandle.swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainHandle.swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = swapChainHandle.swapChainExtent;

        vk::PipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizerInfo = {};
        rasterizerInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterizerInfo.depthClampEnable = VK_FALSE;
        rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizerInfo.lineWidth = 1.0f;
        rasterizerInfo.cullMode = vk::CullModeFlagBits::eNone;
        rasterizerInfo.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizerInfo.depthBiasEnable = VK_FALSE;

        vk::PipelineMultisampleStateCreateInfo multisamplingInfo = {};
        multisamplingInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisamplingInfo.sampleShadingEnable = VK_FALSE;
        multisamplingInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlendingInfo = {};
        colorBlendingInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        colorBlendingInfo.logicOpEnable = VK_FALSE;
        colorBlendingInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendingInfo.attachmentCount = 1;
        colorBlendingInfo.pAttachments = &colorBlendAttachment;

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout.get();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        auto pipelineLayout = logicalDevice->createPipelineLayoutUnique(pipelineLayoutInfo);

        std::array shaderStages = {vertexStageInfo, fragmentStageInfo};

        vk::GraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerInfo;
        pipelineInfo.pMultisampleState = &multisamplingInfo;
        pipelineInfo.pColorBlendState = &colorBlendingInfo;
        pipelineInfo.layout = pipelineLayout.get();
        pipelineInfo.renderPass = renderPass.get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        graphicsPipeline = logicalDevice->createGraphicsPipelineUnique(nullptr, pipelineInfo).value;
        this->pipelineLayout = std::move(pipelineLayout);
    }

    void VulkanRendererConstructor::createFramebuffers() {
        for (auto i = 0u; i < imageResources.size(); i++) {
            vk::FramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.renderPass = renderPass.get();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &swapChainHandle.swapChainImages[i].imageView.get();
            framebufferInfo.width = swapChainHandle.swapChainExtent.width;
            framebufferInfo.height = swapChainHandle.swapChainExtent.height;
            framebufferInfo.layers = 1;
            imageResources[i].framebuffer = logicalDevice->createFramebufferUnique(framebufferInfo);
        }
    }

    void VulkanRendererConstructor::createCommandBuffers() {
        vk::CommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        commandPoolInfo.queueFamilyIndex = graphicsQueue.index;
        commandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        auto commandPool = logicalDevice->createCommandPoolUnique(commandPoolInfo);

        vk::CommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.commandPool = commandPool.get();
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = static_cast<uint32_t>(imageResources.size());

        auto commandBuffers = logicalDevice->allocateCommandBuffersUnique(allocInfo);

        this->commandPool = std::move(commandPool);

        for (auto i = 0u; i < imageResources.size(); i++) {
            imageResources[i].commandBuffer = std::move(commandBuffers[i]);
        }
    }

    void VulkanRendererConstructor::createDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding vpBinding = {};
        vpBinding.binding = 0;
        vpBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        vpBinding.descriptorCount = 1;
        vpBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutBinding modelBinding = {};
        modelBinding.binding = 1;
        modelBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
        modelBinding.descriptorCount = 1;
        modelBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        std::array bindings = {vpBinding, modelBinding};

        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
        descriptorSetLayoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
        descriptorSetLayoutInfo.bindingCount = bindings.size();
        descriptorSetLayoutInfo.pBindings = bindings.data();

        descriptorSetLayout = logicalDevice->createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);
    }

    void VulkanRendererConstructor::createDescriptorPool() {
        vk::DescriptorPoolSize viewProjectionPoolSize = {};
        viewProjectionPoolSize.type = vk::DescriptorType::eUniformBuffer;
        viewProjectionPoolSize.descriptorCount = static_cast<uint32_t>(imageResources.size());

        vk::DescriptorPoolSize modelPoolSize = {};
        modelPoolSize.type = vk::DescriptorType::eUniformBufferDynamic;
        modelPoolSize.descriptorCount = static_cast<uint32_t>(imageResources.size());

        std::array poolSizes = {viewProjectionPoolSize, modelPoolSize};

        vk::DescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
        descriptorPoolInfo.poolSizeCount = poolSizes.size();
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = static_cast<uint32_t>(imageResources.size());

        descriptorPool = logicalDevice->createDescriptorPoolUnique(descriptorPoolInfo);
    }

    void VulkanRendererConstructor::allocateDescriptorSets() {
        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
        descriptorSetAllocateInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool.get();
        descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(imageResources.size());

        std::vector descriptorSetLayouts(imageResources.size(), descriptorSetLayout.get());
        descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

        auto sets = logicalDevice->allocateDescriptorSetsUnique(descriptorSetAllocateInfo);
        for (auto i = 0u; i < imageResources.size(); i++) {
            imageResources[i].descriptorSet = std::move(sets[i]);
        }
    }

    void VulkanRendererConstructor::createViewProjectionBuffer(ImageResources &resource) {
        auto createResult = createBuffer(
            {physicalDevice, logicalDevice.get()},
            sizeof(ViewProjection),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        resource.viewProjectionUniformBuffer = std::move(createResult.buffer);
        resource.viewProjectionUniformBufferMemory = std::move(createResult.memory);
    }

    void VulkanRendererConstructor::createModelBuffer(int maxObjectsPerFrame, ImageResources &resource) {
        auto modelAlignment = getAlignmentSizeForType(physicalDevice, sizeof(Model));

        auto modelTransferBuffer = createBuffer(
            {physicalDevice, logicalDevice.get()},
            modelAlignment * maxObjectsPerFrame,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        );

        resource.modelTransferBuffer = std::move(modelTransferBuffer.buffer);
        resource.modelTransferBufferMemory = std::move(modelTransferBuffer.memory);
    }

    void VulkanRendererConstructor::allocateModelBufferResources(int maxObjectsPerFrame) {
        auto modelAlignment = getAlignmentSizeForType(physicalDevice, sizeof(Model));

        modelTransferSpace = static_cast<Model *>(aligned_alloc(modelAlignment, sizeof(Model) * maxObjectsPerFrame));
    }

    void VulkanRendererConstructor::createUniformBuffers(int maxObjectsPerFrame) {
        for (auto &resource: imageResources) {
            createViewProjectionBuffer(resource);
            createModelBuffer(maxObjectsPerFrame, resource);
        }
        allocateModelBufferResources(maxObjectsPerFrame);
    }

    void VulkanRendererConstructor::bindDescriptorsToBuffers() {
        for (auto &resource: imageResources) {
            vk::DescriptorBufferInfo viewProjectionBufferInfo = {};
            viewProjectionBufferInfo.buffer = resource.viewProjectionUniformBuffer.get();
            viewProjectionBufferInfo.offset = 0;
            viewProjectionBufferInfo.range = sizeof(ViewProjection);

            vk::WriteDescriptorSet writeViewProjectionDescriptorSet = {};
            writeViewProjectionDescriptorSet.sType = vk::StructureType::eWriteDescriptorSet;
            writeViewProjectionDescriptorSet.dstSet = resource.descriptorSet.get();
            writeViewProjectionDescriptorSet.dstBinding = 0;
            writeViewProjectionDescriptorSet.dstArrayElement = 0;
            writeViewProjectionDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
            writeViewProjectionDescriptorSet.descriptorCount = 1;
            writeViewProjectionDescriptorSet.pBufferInfo = &viewProjectionBufferInfo;

            vk::DescriptorBufferInfo modelBufferInfo = {};
            modelBufferInfo.buffer = resource.modelTransferBuffer.get();
            modelBufferInfo.offset = 0;
            modelBufferInfo.range = sizeof(Model);

            vk::WriteDescriptorSet writeModelDescriptorSet = {};
            writeModelDescriptorSet.sType = vk::StructureType::eWriteDescriptorSet;
            writeModelDescriptorSet.dstSet = resource.descriptorSet.get();
            writeModelDescriptorSet.dstBinding = 1;
            writeModelDescriptorSet.dstArrayElement = 0;
            writeModelDescriptorSet.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
            writeModelDescriptorSet.descriptorCount = 1;
            writeModelDescriptorSet.pBufferInfo = &modelBufferInfo;

            std::array writeDescriptorSets = {writeViewProjectionDescriptorSet, writeModelDescriptorSet};

            logicalDevice->updateDescriptorSets(writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
        }
    }

    void VulkanRendererConstructor::createDescriptorSets(int maxObjectsPerFrame) {
        createDescriptorPool();
        createDescriptorSetLayout();
        allocateDescriptorSets();
        createUniformBuffers(maxObjectsPerFrame);
        bindDescriptorsToBuffers();
    }

    void VulkanRendererConstructor::createFrameSyncs(int maxFramesInFlight) {
        vk::SemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
        frameSyncs.resize(maxFramesInFlight);

        vk::FenceCreateInfo fenceInfo = {};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (auto &frameSync: frameSyncs) {
            frameSync.imageAvailableSemaphore = logicalDevice->createSemaphoreUnique(semaphoreInfo);
            frameSync.renderFinishedSemaphore = logicalDevice->createSemaphoreUnique(semaphoreInfo);
            frameSync.inFlightFence = logicalDevice->createFenceUnique(fenceInfo);
        }
    }

    void VulkanRendererConstructor::createImageResources(int maxObjectsPerFrame) {
        std::vector<ImageResources> imageResources;
        imageResources.resize(swapChainHandle.swapChainImages.size());
        this->imageResources = std::move(imageResources);

        createFramebuffers();
        createCommandBuffers();
        createDescriptorSets(maxObjectsPerFrame);
    }

    std::unique_ptr<VulkanRenderer> VulkanRendererConstructor::create(GLFWwindow *window, int maxObjectsPerFrame,
                                                                      int maxFramesInFlight) {
        if (alreadyUsed) {
            throw std::runtime_error("VulkanRendererConstructor can only be used once");
        }

        this->window = window;
        instance = createInstance();
        createSurface();
        retrievePhysicalDevice();
        createLogicalDevice();
        initQueues();
        createSwapChain();
        createRenderPass();
        createImageResources(maxObjectsPerFrame);
        createGraphicsPipeline();
        createFrameSyncs(maxFramesInFlight);

        alreadyUsed = true;

        return std::make_unique<VulkanRenderer>(
            RendererResources{
                window,
                std::move(instance),
                physicalDevice,
                std::move(logicalDevice),
                std::move(surface),
                std::move(swapChainHandle),
                graphicsQueue,
                presentQueue,
                std::move(renderPass),
                std::move(graphicsPipeline),
                std::move(pipelineLayout),
                std::move(commandPool),
                std::move(imageResources),
                std::move(frameSyncs),
                std::move(descriptorPool),
                std::move(descriptorSetLayout),
                modelTransferSpace
            },
            maxObjectsPerFrame
        );
    }

    void VulkanRendererConstructor::createSurface() {
        VkSurfaceKHR glfwSurface;

        VK_CHECK(glfwCreateWindowSurface(instance.get(), window, nullptr, &glfwSurface));

        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(instance.get());

        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(glfwSurface), deleter);
    }

    bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    void VulkanRendererConstructor::retrievePhysicalDevice() {
        auto devices = instance->enumeratePhysicalDevices();

        auto filtered = std::views::filter(devices, [this](const auto &device) {
            return isDeviceSuitable(device, surface.get());
        });

        if (filtered.begin() == filtered.end()) {
            throw std::runtime_error("No suitable device found");
        }

        physicalDevice = filtered.front();
    }

    std::vector<const char *> getExtensionsToEnable(vk::PhysicalDevice physicalDevice) {
        auto resultingExtensions = std::vector<const char *>();

        auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        for (const auto &extension: requiredExtensions) {
            resultingExtensions.push_back(extension);
        }

        if (std::ranges::find_if(availableExtensions, [](const auto &extension) {
            return strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0;
        }) != availableExtensions.end()) {
            resultingExtensions.emplace_back("VK_KHR_portability_subset");
        }

        return resultingExtensions;
    }

    QueueHandles getRawQueueHandles(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        QueueHandles queueHandles;

        auto queueFamilyProperties = device.getQueueFamilyProperties();

        for (auto i = 0; i < queueFamilyProperties.size(); i++) {
            const auto &queueFamily = queueFamilyProperties[i];
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                queueHandles.graphicsQueue.index = i;
            }

            if (device.getSurfaceSupportKHR(i, surface)) {
                queueHandles.presentQueue.index = i;
            }

            if (queueHandles.graphicsQueue.index >= 0 && queueHandles.presentQueue.index >= 0) {
                break;
            }
        }

        return queueHandles;
    }

    std::vector<VkDeviceQueueCreateInfo> makeQueueCreateInfos(const QueueHandles &queueHandles) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        for (const auto &queueHandle: {queueHandles.graphicsQueue, queueHandles.presentQueue}) {
            if (uniqueQueueFamilies.contains(queueHandle.index)) {
                continue;
            }
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueHandle.index;
            queueCreateInfo.queueCount = 1;
            constexpr float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
            uniqueQueueFamilies.insert(queueHandles.graphicsQueue.index);
        }

        return queueCreateInfos;
    }

    void VulkanRendererConstructor::createLogicalDevice() {
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;

        auto enabledExtensions = getExtensionsToEnable(physicalDevice);
        createInfo.enabledExtensionCount = enabledExtensions.size();
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();

        auto features = VkPhysicalDeviceFeatures{};
        createInfo.pEnabledFeatures = &features;

        auto queueCrateInfos = makeQueueCreateInfos(getRawQueueHandles(physicalDevice, surface.get()));
        createInfo.queueCreateInfoCount = queueCrateInfos.size();
        createInfo.pQueueCreateInfos = queueCrateInfos.data();

        logicalDevice = physicalDevice.createDeviceUnique(createInfo);
    }

    vk::SurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
        if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
            return {*preferableFormats.begin(), preferableColorSpace};
        }

        for (const auto &format: availableFormats) {
            if (preferableFormats.contains(format.format) && format.colorSpace == preferableColorSpace) {
                return format;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR chooseBestPresentMode(const std::vector<vk::PresentModeKHR> &presentModes) {
        for (const auto &mode: presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                return mode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities, GLFWwindow *window) {
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            return surfaceCapabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        vk::Extent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        extent.width = std::clamp(extent.width, surfaceCapabilities.minImageExtent.width,
                                  surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surfaceCapabilities.minImageExtent.height,
                                   surfaceCapabilities.maxImageExtent.height);

        return extent;
    }

    vk::UniqueImageView createImageView(vk::Device device, vk::Image image, vk::Format format,
                                        vk::ImageAspectFlagBits eColor) {
        vk::ImageViewCreateInfo createInfo{};
        createInfo.sType = vk::StructureType::eImageViewCreateInfo;
        createInfo.pNext = nullptr;
        createInfo.image = image;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = format;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        return device.createImageViewUnique(createInfo);
    }

    void VulkanRendererConstructor::createSwapChain() {
        auto swapChainDetails = getSwapChainDetails(physicalDevice, surface.get());

        auto bestFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
        auto bestPresentMode = chooseBestPresentMode(swapChainDetails.presentModes);
        auto extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities, window);

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
        createInfo.pNext = nullptr;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        createInfo.surface = surface.get();
        createInfo.clipped = VK_TRUE;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        createInfo.imageExtent = extent;
        createInfo.imageFormat = bestFormat.format;
        createInfo.imageColorSpace = bestFormat.colorSpace;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        createInfo.clipped = VK_TRUE;
        createInfo.presentMode = bestPresentMode;
        const auto minSwapChainImages = std::min(
            swapChainDetails.surfaceCapabilities.minImageCount + 1,
            swapChainDetails.surfaceCapabilities.maxImageCount
        );
        createInfo.minImageCount = minSwapChainImages;
        createInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;

        if (graphicsQueue.index != presentQueue.index) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            const uint32_t queueFamilyIndices[] = {
                static_cast<uint32_t>(graphicsQueue.index),
                static_cast<uint32_t>(presentQueue.index)
            };
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }

        auto swapChain = logicalDevice->createSwapchainKHRUnique(createInfo);
        std::vector<SwapChainImage> swapChainImages;
        for (
            auto rawSwapChainImages = logicalDevice->getSwapchainImagesKHR(swapChain.get());
            const auto &rawSwapChainImage: rawSwapChainImages
        ) {
            auto imageView = createImageView(
                logicalDevice.get(),
                rawSwapChainImage,
                bestFormat.format,
                vk::ImageAspectFlagBits::eColor
            );

            swapChainImages.push_back({
                rawSwapChainImage,
                std::move(imageView)
            });
        }

        this->swapChainHandle = {
            std::move(swapChain),
            bestFormat.format,
            extent,
            std::move(swapChainImages)
        };
    }

    bool hasRequiredExtensions(const vk::PhysicalDevice &device) {
        auto availableExtensions = device.enumerateDeviceExtensionProperties();

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

    bool hasRequiredQueueFamilies(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        return getRawQueueHandles(device, surface).areIndicesValid();
    }

    SwapChainDetails getSwapChainDetails(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        SwapChainDetails details;

        details.surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);
        if (details.surfaceCapabilities.maxImageCount == 0) {
            details.surfaceCapabilities.maxImageCount = UINT32_MAX;
        }

        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);

        return details;
    }

    bool hasValidSwapChain(const vk::PhysicalDevice &device, vk::SurfaceKHR surface) {
        auto details = getSwapChainDetails(device, surface);
        return !details.formats.empty() && !details.presentModes.empty();
    }

    bool isDeviceSuitable(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
        return hasRequiredExtensions(device)
               && hasRequiredQueueFamilies(device, surface)
               && hasValidSwapChain(device, surface);
    }


    std::vector<const char *> getRequiredExtensions();

    VkApplicationInfo getAppInfo();

    vk::UniqueInstance createInstance() {
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.flags = 0;

        auto appInfo = getAppInfo();
        info.pApplicationInfo = &appInfo;

        const char *layers[] = {
            "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_profiles"
        };
        info.enabledLayerCount = 2;
        info.ppEnabledLayerNames = layers;

        auto extensions = getRequiredExtensions();
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();

        return vk::createInstanceUnique(info);
    }

    VkApplicationInfo getAppInfo() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Test";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        return appInfo;
    }

    std::vector<const char *> getRequiredExtensions() {
        auto extensions = std::vector<const char *>();
        uint32_t glfwExtensionCount;

        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (auto i = 0u; i < glfwExtensionCount; i++) {
            extensions.push_back(glfwExtensions[i]);
        }

        return extensions;
    }
}

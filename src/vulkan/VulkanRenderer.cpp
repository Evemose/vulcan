#include "VulkanRenderer.h"

#include "VulkanRendererConstructor.h"

namespace enjine {
    void VulkanRenderer::submitBuffer(unsigned imageIndex) const {
        vk::SubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &resources.frameSyncs[currentFrame].imageAvailableSemaphore.get();
        submitInfo.sType = vk::StructureType::eSubmitInfo;
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &resources.imageResources[imageIndex].commandBuffer.get();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &resources.frameSyncs[currentFrame].renderFinishedSemaphore.get();

        resources.graphicsQueue.queue.submit(submitInfo, resources.frameSyncs[currentFrame].inFlightFence.get());
    }

    void VulkanRenderer::present(uint32_t imageIndex) const {
        vk::PresentInfoKHR presentInfo;
        presentInfo.sType = vk::StructureType::ePresentInfoKHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &resources.frameSyncs[currentFrame].renderFinishedSemaphore.get();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &resources.swapChainHandle.swapChain.get();
        presentInfo.pImageIndices = &imageIndex;

        auto result = resources.presentQueue.queue.presentKHR(presentInfo);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present");
        }

    }

    void VulkanRenderer::render(const std::vector<RenderObject> &meshes) {

        auto fence = resources.frameSyncs[currentFrame].inFlightFence.get();

        auto result = resources.logicalDevice->waitForFences(1, &fence, VK_TRUE, UINT64_MAX);

        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to wait for fence");
        }

        resources.logicalDevice->resetFences({ fence });

        auto imageIndex = resources.logicalDevice->acquireNextImageKHR(
            resources.swapChainHandle.swapChain.get(),
            UINT64_MAX,
            resources.frameSyncs[currentFrame].imageAvailableSemaphore.get()
        ).value;

        recordDrawCommand(imageIndex, meshes);
        submitBuffer(imageIndex);
        present(imageIndex);

        currentFrame = (currentFrame + 1) % static_cast<int>(maxFramesInFlight);

    }

    void VulkanRenderer::recordDrawCommand(uint32_t imageIndex, const std::vector<RenderObject> &objects) const {
        vk::RenderPassBeginInfo renderPassInfo;
        renderPassInfo.renderPass = resources.renderPass.get();
        renderPassInfo.framebuffer = resources.imageResources[imageIndex].framebuffer.get();
        renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
        renderPassInfo.renderArea.extent = resources.swapChainHandle.swapChainExtent;

        auto clearValues = std::array{
            vk::ClearValue{std::array{0.6f, 0.65f, 0.4f, 1.0f}}
        };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        auto commandBuffer = resources.imageResources[imageIndex].commandBuffer.get();

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

        commandBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

        commandBuffer.begin(beginInfo);

        commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, resources.graphicsPipeline.get());

        // for (const auto &object: objects) {
        //     commandBuffer.draw(
        //         object.mesh.vertices.size(),
        //         1,
        //         0,
        //         0
        //     );
        // }

        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();

        commandBuffer.end();
    }
}

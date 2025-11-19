#include "ikRenderer.hpp"




//std
#include <stdexcept>
#include <cassert>
#include <array>
namespace ikE {



	IkeRenderer::IkeRenderer(IkeWindow& window, IkeDeviceEngine& device) : ikeWindow(window), ikeDevice(device) {

		recreateSwapChain(),
		createCommandBuffers();
	}

	IkeRenderer::~IkeRenderer() { freeCommandBuffers(); }




	void IkeRenderer::recreateSwapChain() {
		auto extent = ikeWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = ikeWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(ikeDevice.device());
		if (ikSwapChain == nullptr) {
			ikSwapChain = std::make_unique<ikEngineSwapChain>(ikeDevice, extent);
		}
		else {
			std::shared_ptr<ikEngineSwapChain> oldSwapChain = std::move(ikSwapChain);
			ikSwapChain = std::make_unique<ikEngineSwapChain>(ikeDevice, extent, oldSwapChain);
			
			if (!oldSwapChain->compareSwapFormats(*ikSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}

			
		}
		//we will come back to this


	}



	void ikE::IkeRenderer::createCommandBuffers() {
		commandBuffers.resize(ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = ikeDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(ikeDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}


	void IkeRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(ikeDevice.device(),
			ikeDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}


	


	VkCommandBuffer IkeRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress!");
	;
		auto result = ikSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image !");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffers !");

		}
		return commandBuffer;
	}


	void IkeRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress!");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = ikSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ikeWindow.wasWindowResized()) {
			ikeWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			     throw std::runtime_error("failed to present swap chain image !");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT;
	}



	void IkeRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && " Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() && " Can't beging render pass on command buffer from a different frame"
		);


		//this Describe the render pass you want to begin
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = ikSwapChain->getRenderPass();
		renderPassInfo.framebuffer = ikSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = ikSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f,0.01f,0.01f,1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 }; // instead of 1.0f,0.0f
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		//we record to the command buffer to begin the renderPass
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ikSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(ikSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, ikSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


	}


	void IkeRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && " Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() && " Can't end render pass on command buffer from a different frame"
		);
		// End the render pass and CommandBuffer
		vkCmdEndRenderPass(commandBuffer);

	}

}//namespace



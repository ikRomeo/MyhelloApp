#include "First_App.hpp"

//std
#include <stdexcept>
#include <array>
namespace ikE {
	FirstApp::FirstApp() {ikeDeviceEngine.createCommandPool(), createPipelinelayout(), createPipeline(), createCommandBuffers(); }
	
	FirstApp::~FirstApp() { vkDestroyPipelineLayout(ikeDeviceEngine.device(), pipelineLayout, nullptr); }


	void FirstApp::run() {
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(ikeDeviceEngine.device());
	}
	//Note to be explained
	/* a pipeline SetLayouts is used to pass data other than vertex data to the fragment and vertex shaders
	 which can include textures and uniform buffer objects
	 pushConstant are ways to send small amount of data efficiently to the shader progams which is either vertex or fragment shaders
	 */
	void FirstApp::createPipelinelayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(ikeDeviceEngine.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	//Note to be explained
	/* here we call the ikePipeline class with its member function and then its arguments is ikEngineswapChain class that has member functions width and height
	   we able to do this because in firstApp.hpp we have them as an include file
	
	*/
	void FirstApp::createPipeline() {
		auto pipelineConfig = ikePipeline::defaultPipelineConfigInfo(ikEngineSwapChain.width(), ikEngineSwapChain.height());
		pipelineConfig.renderPass = ikEngineSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		//Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "frag.spv", "vert.spv", pipelineConfig);

		Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "vert.spv", "frag.spv", pipelineConfig);
	}

	void ikE::FirstApp::createCommandBuffers() {
		commandBuffers.resize(ikEngineSwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = ikeDeviceEngine.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(ikeDeviceEngine.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
		// we need to record the draw commands to each buffer
		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


			if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffers !");

			}
			//we create another local variable this time with type vk render pass begin info
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = ikEngineSwapChain.getRenderPass();
			renderPassInfo.framebuffer = ikEngineSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = ikEngineSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f,0.1f,0.1f,1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 }; // instead of 1.0f,0.0f
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			//we record to the command buffer to begin the renderPass
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			Pipeline->bind(commandBuffers[i]);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}

	}

	void ikE::FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = ikEngineSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image !");
		}
		result = ikEngineSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image !");
		}
	}






}//namespace ikE
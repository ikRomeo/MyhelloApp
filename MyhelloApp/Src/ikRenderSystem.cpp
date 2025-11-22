#include "ikRenderSystem.hpp"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



//std
#include <stdexcept>
#include <cassert>
#include <array>
namespace ikE {

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	//FirstApp::FirstApp() {loadGameObjects(),ikeDeviceEngine.createCommandPool(), createPipelinelayout(); }
	IkRenderSystem::IkRenderSystem(IkeDeviceEngine& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : ikeDeviceEngine(device){
		 createPipelinelayout(globalSetLayout),
		 createPipeline(renderPass);
	}

	IkRenderSystem::~IkRenderSystem() { vkDestroyPipelineLayout(ikeDeviceEngine.device(), pipelineLayout, nullptr); }







	void IkRenderSystem::createPipelinelayout(VkDescriptorSetLayout globalSetLayout) {
		//need explanation
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(ikeDeviceEngine.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	//Note to be explained
	/* here we call the ikePipeline class with its member function and then its arguments is ikEngineswapChain class that has member functions width and height
	   we able to do this because in firstApp.hpp we have them as an include file

	*/
	void IkRenderSystem::createPipeline(VkRenderPass renderPass) {

		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


		PipelineConfigInfo pipelineConfig{};
		ikePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		//Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "frag.spv", "vert.spv", pipelineConfig);

		Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "Shaders/vert.spv", "Shaders/frag.spv", pipelineConfig);
	}

	//needs explanation
	void IkRenderSystem::renderGameObjects(FrameInfo &frameInfo) {
		Pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr );


		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}


}//namespace ikE
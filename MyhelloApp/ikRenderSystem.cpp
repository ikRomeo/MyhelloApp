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
		glm::mat4 transform{ 1.f };
		alignas(16) glm::vec3 color;
	};

	//FirstApp::FirstApp() {loadGameObjects(),ikeDeviceEngine.createCommandPool(), createPipelinelayout(); }
	IkRenderSystem::IkRenderSystem(IkeDeviceEngine& device, VkRenderPass renderPass) : ikeDeviceEngine(device){
		 createPipelinelayout(), 
		 createPipeline(renderPass); 
	}

	IkRenderSystem::~IkRenderSystem() { vkDestroyPipelineLayout(ikeDeviceEngine.device(), pipelineLayout, nullptr); }







	void IkRenderSystem::createPipelinelayout() {
		//need explanation
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
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

		Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "vert.spv", "frag.spv", pipelineConfig);
	}

	//needs explanation
	void IkRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<IkgameObject>& gameObjects, const IkCamera& camera) {
		Pipeline->bind(commandBuffer);

		auto projectionView = camera.getProjection() * camera.getView();


		for (auto& obj : gameObjects) {
			// this rotates the triangle
			obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
			obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());



			SimplePushConstantData push{};
			push.color = obj.color;
			push.transform = projectionView * obj.transform.mat4();

			vkCmdPushConstants(commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}


}//namespace ikE
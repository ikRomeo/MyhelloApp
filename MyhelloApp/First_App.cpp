#include "First_App.hpp"
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
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	//FirstApp::FirstApp() {loadGameObjects(),ikeDeviceEngine.createCommandPool(), createPipelinelayout(); }
	FirstApp::FirstApp() { loadGameObjects(), createPipelinelayout(),createPipeline(); }
	FirstApp::~FirstApp() { vkDestroyPipelineLayout(ikeDeviceEngine.device(), pipelineLayout, nullptr); }

	void FirstApp::run() {
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = IkRenderer.beginFrame()) {
				// begin offscreen shadow pass
				// render shadow casting objects
				// end offscreen shadow pass
				IkRenderer.beginSwapChainRenderPass(commandBuffer);
				renderGameObjects(commandBuffer);
				IkRenderer.endSwapChainRenderPass(commandBuffer);
				IkRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(ikeDeviceEngine.device());
	}



	//here we load the vertices via ikEnginModel
	void FirstApp::loadGameObjects() {
		std::vector<ikEngineModel::Vertex> vertices{
			{{0.0f,-0.5f}, {1.0f,0.0f,0.0f}},
			{{0.5f,0.5f},  {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}

		};
		
		//ikModel = std::make_unique<ikEngineModel>(ikeDeviceEngine, vertices);
		 auto ikModel = std::make_shared<ikE::ikEngineModel>(ikeDeviceEngine, vertices);

		 auto triangle = IkgameObject::createGameObject();
		 triangle.model = ikModel;
		 triangle.color = { .1f, .8, .1f };
		 triangle.transform2d.translation.x = .2f;
		 triangle.transform2d.scale = { 2.f, .5f };
		 triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		 gameObjects.push_back(std::move(triangle));

	}



	//Note to be explained
	/* a pipeline SetLayouts is used to pass data other than vertex data to the fragment and vertex shaders
	 which can include textures and uniform buffer objects
	 pushConstant are ways to send small amount of data efficiently to the shader progams which is either vertex or fragment shaders
	 */
	void FirstApp::createPipelinelayout() {
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
	void FirstApp::createPipeline() {
		
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


		PipelineConfigInfo pipelineConfig{};
		ikePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = IkRenderer.getSwapChainRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		//Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "frag.spv", "vert.spv", pipelineConfig);

		Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "vert.spv", "frag.spv", pipelineConfig);
	}

	



	





	//needs explanation
	void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer) {
		Pipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {

			// this rotates the triangle
			obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.01f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();

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
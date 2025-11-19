#include "First_App.hpp"
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

	

	FirstApp::FirstApp() { loadGameObjects(); }
	FirstApp::~FirstApp() { }

	void FirstApp::run() {
		IkRenderSystem ikeRenderSystem{ ikeDeviceEngine,IkRenderer.getSwapChainRenderPass() };
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();

			if (auto commandBuffer = IkRenderer.beginFrame()) {
				// begin offscreen shadow pass
				// render shadow casting objects
				// end offscreen shadow pass
				IkRenderer.beginSwapChainRenderPass(commandBuffer);
				ikeRenderSystem.renderGameObjects(commandBuffer,gameObjects);
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
		
		
		 auto ikModel = std::make_shared<ikE::ikEngineModel>(ikeDeviceEngine, vertices);

		 auto triangle = IkgameObject::createGameObject();
		 triangle.model = ikModel;
		 triangle.color = { .1f, .8, .1f };
		 triangle.transform2d.translation.x = .2f;
		 triangle.transform2d.scale = { 2.f, .5f };
		 triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		 gameObjects.push_back(std::move(triangle));

	}



	

}//namespace ikE
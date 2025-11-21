#include "First_App.hpp"
#include "KeyBoardMovementController.hpp"
#include "ikCamera.hpp"
#include "ikRenderSystem.hpp"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



//std

#include <array>
#include <chrono>
#include <stdexcept>
#include <cassert>

namespace ikE {

	

	FirstApp::FirstApp() { loadGameObjects(); }
	FirstApp::~FirstApp() { }

	void FirstApp::run() {
		IkRenderSystem ikeRenderSystem{ ikeDeviceEngine,IkRenderer.getSwapChainRenderPass() };
        IkCamera camera{};
        //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = IkgameObject::createGameObject();
        keyBoardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();


            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            //THIS needs explanation
            //frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.moveInPlaneXZ(ikeWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);


            float aspect = IkRenderer.getAspectRatio();
            // camera.setOrthographicProjection( -aspect, aspect , -1, 1,  -1, 1 );
            camera.setPerspectiveProjection(glm::radians( 50.f), aspect, 0.1f, 10.f);


			if (auto commandBuffer = IkRenderer.beginFrame()) {
				// begin offscreen shadow pass
				// render shadow casting objects
				// end offscreen shadow pass
				IkRenderer.beginSwapChainRenderPass(commandBuffer);
				ikeRenderSystem.renderGameObjects(commandBuffer,gameObjects,camera);
				IkRenderer.endSwapChainRenderPass(commandBuffer);
				IkRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(ikeDeviceEngine.device());
	}




 


	//here we load the vertices via ikEnginModel
	void FirstApp::loadGameObjects() {
		std::shared_ptr<ikEngineModel> ikModel = ikEngineModel::createModelFromFile(ikeDeviceEngine, "Assets/models/flat_vase.obj");

        auto flatVase= IkgameObject::createGameObject();
        flatVase.model = ikModel;
        flatVase.transform.translation = { -.5f, .5f, 2.5f };
		flatVase.transform.scale = { 3.f ,1.5f,3.f};
        gameObjects.push_back(std::move(flatVase));  
		
		
	

	    ikModel = ikEngineModel::createModelFromFile(ikeDeviceEngine, "Assets/models/smooth_vase.obj");

		auto smoothVase = IkgameObject::createGameObject();
		smoothVase.model = ikModel;
		smoothVase.transform.translation = { .5f, .5f, 2.5f };
		smoothVase.transform.scale = { 3.f ,1.5f,3.f };
		gameObjects.push_back(std::move(smoothVase));


	}

	

}//namespace ikE
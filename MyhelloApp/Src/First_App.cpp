#include "First_App.hpp"
#include "KeyBoardMovementController.hpp"
#include "ikBuffer.hpp"
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
#include <numeric>






namespace ikE {

    struct GlobalUbo {
	    glm::mat4 projectionView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		glm::vec3 lightPosition{ -1.f };
		alignas (16)glm::vec4 lightColor{ 1.f }; // w is light intensity
    }; 
	

	FirstApp::FirstApp() { 
		globalPool =
			IkDescriptorPool::Builder(ikeDeviceEngine)
			.setMaxSets(ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects(); 
	}

	FirstApp::~FirstApp() { }

	void FirstApp::run() {

		std::vector < std::unique_ptr<IkBuffer>> uboBuffers(ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<IkBuffer>(
				ikeDeviceEngine,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			uboBuffers[i]->map();
		}


		auto globalSetLayout = IkDescriptorSetLayout::Builder(ikeDeviceEngine)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(ikEngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			IkDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		IkRenderSystem ikeRenderSystem{ 
			ikeDeviceEngine,
			IkRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout()};
        IkCamera camera{};


        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = IkgameObject::createGameObject();

		viewerObject.transform.translation.z = -2.5f;

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
            camera.setPerspectiveProjection(glm::radians( 50.f), aspect, 0.1f, 1000.f); // 1000 is for clippin or 100


			if (auto commandBuffer = IkRenderer.beginFrame()) {
				int frameIndex = IkRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};
				//update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();




				//render
				IkRenderer.beginSwapChainRenderPass(commandBuffer);
				ikeRenderSystem.renderGameObjects(frameInfo,gameObjects);
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
        flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 3.f ,1.5f,3.f};
        gameObjects.push_back(std::move(flatVase));  
		
		
	

	    ikModel = ikEngineModel::createModelFromFile(ikeDeviceEngine, "Assets/models/smooth_vase.obj");

		auto smoothVase = IkgameObject::createGameObject();
		smoothVase.model = ikModel;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f ,1.5f,3.f };
		gameObjects.push_back(std::move(smoothVase));



		ikModel = ikEngineModel::createModelFromFile(ikeDeviceEngine, "Assets/models/quad.obj");

		auto floor = IkgameObject::createGameObject();
		floor.model = ikModel;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f ,1.f,3.f };
		gameObjects.push_back(std::move(floor));


	}

	

}//namespace ikE
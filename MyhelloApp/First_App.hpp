#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP


#include "ikDeviceEngine.hpp"
#include "ikPipeline.hpp"
#include "ikSwapChain.hpp"
#include "ikWindow.hpp"

//std
#include <memory>
#include <vector>
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		//because we have the constructors here we should also remember to delete the copy constructors 
		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator =(const FirstApp&) = delete;

		void run();

	private:
		//new functions which needs explanation
		
		void createPipelinelayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();


		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
		//we take the ikeWindow here after we create createShaderModule
		IkeDeviceEngine ikeDeviceEngine{ ikeWindow };
		ikEngineSwapChain ikEngineSwapChain{ ikeDeviceEngine,ikeWindow.getExtent() };

		// we councel the ikePipeline struct and use an unique pointer instead
        //ikePipeline ikePipeline{ikeDeviceEngine,"frag.spv","vert.spv",ikePipeline::defaultPipelineConfigInfo(WIDTH,HEIGTH)};
		std::unique_ptr<ikePipeline> Pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
	};

} //namepace
#endif //header guard

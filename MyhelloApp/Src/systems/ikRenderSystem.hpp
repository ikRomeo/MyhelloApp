#ifndef IKRENDERSYSTEM_HPP
#define IKRENDERSYSTEM_HPP

#include "../ikCamera.hpp"
#include "../ikDeviceEngine.hpp"
#include "../ikgameObject.hpp"
#include "../ikPipeline.hpp"
#include "../ikframeInfo.hpp"

//std
#include <memory>
#include <vector>
namespace ikE {
	class IkRenderSystem {
	public:
		

		//because we have the constructors here we should also remember to delete the copy constructors 
		IkRenderSystem(IkeDeviceEngine &device, VkRenderPass renderPass,VkDescriptorSetLayout globalSetLayout);
		~IkRenderSystem();

		IkRenderSystem(const IkRenderSystem&) = delete;
		IkRenderSystem& operator =(const IkRenderSystem&) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

	private:
		
	
		void createPipelinelayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		



		IkeDeviceEngine &ikeDeviceEngine;
		



		std::unique_ptr<ikePipeline> Pipeline;
		VkPipelineLayout pipelineLayout;
		
	};

} //namepace
#endif //header guard

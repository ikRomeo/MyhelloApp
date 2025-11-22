#ifndef IKPOINTLIGHTSYSTEM_HPP
#define IKPOINTLIGHTSYSTEM_HPP

#include "../ikCamera.hpp"
#include "../ikDeviceEngine.hpp"
#include "../ikgameObject.hpp"
#include "../ikPipeline.hpp"
#include "../ikframeInfo.hpp"

//std
#include <memory>
#include <vector>
namespace ikE {
	class IkPointLightSystem {
	public:
		

		//because we have the constructors here we should also remember to delete the copy constructors 
		IkPointLightSystem(IkeDeviceEngine &device, VkRenderPass renderPass,VkDescriptorSetLayout globalSetLayout);
		~IkPointLightSystem();

		IkPointLightSystem(const IkPointLightSystem&) = delete;
		IkPointLightSystem& operator =(const IkPointLightSystem&) = delete;

        void render(FrameInfo &frameInfo);

	private:
		
	
		void createPipelinelayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
		



		IkeDeviceEngine &ikeDeviceEngine;
		



		std::unique_ptr<ikePipeline> Pipeline;
		VkPipelineLayout pipelineLayout;
		
	};

} //namepace
#endif //header guard

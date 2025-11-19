#ifndef IKRENDERSYSTEM_HPP
#define IKRENDERSYSTEM_HPP


#include "ikDeviceEngine.hpp"
#include "ikgameObject.hpp"
#include "ikPipeline.hpp"

//std
#include <memory>
#include <vector>
namespace ikE {
	class IkRenderSystem {
	public:
		

		//because we have the constructors here we should also remember to delete the copy constructors 
		IkRenderSystem(IkeDeviceEngine &device, VkRenderPass renderPass);
		~IkRenderSystem();

		IkRenderSystem(const IkRenderSystem&) = delete;
		IkRenderSystem& operator =(const IkRenderSystem&) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<IkgameObject> &gameObjects);

	private:
		
	
		void createPipelinelayout();
		void createPipeline(VkRenderPass renderPass);
		



		IkeDeviceEngine &ikeDeviceEngine;
		



		std::unique_ptr<ikePipeline> Pipeline;
		VkPipelineLayout pipelineLayout;
		
	};

} //namepace
#endif //header guard

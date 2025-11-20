#ifndef IKRENDERER_APP_HPP
#define IKRENDERER_APP_HPP


#include "ikDeviceEngine.hpp"

#include "ikSwapChain.hpp"
#include "ikWindow.hpp"


//std
#include <cassert>
#include <memory>
#include <vector>
namespace ikE {
	class IkeRenderer {
	public:
		

		//because we have the constructors here we should also remember to delete the copy constructors 
		IkeRenderer(IkeWindow &window, IkeDeviceEngine &device);
		~IkeRenderer();

		IkeRenderer(const IkeRenderer&) = delete;
		IkeRenderer &operator =(const IkeRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const {
			return ikSwapChain->getRenderPass();}
		float getAspectRatio() const { return ikSwapChain->extentAspectRatio(); }

		bool isFrameInProgress() const { return isFrameStarted; };


		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress!");
			return commandBuffers[currentFrameIndex]; }

		int getFrameIndex() const {
			assert(isFrameStarted && "Can not get frame index when frame is not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		//new functions which needs explanation
		//void loadModels();
	
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
	

		IkeWindow  &ikeWindow;
		IkeDeviceEngine &ikeDevice;
	
		std::unique_ptr<ikEngineSwapChain> ikSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	
	};

} //namepace
#endif //header guard
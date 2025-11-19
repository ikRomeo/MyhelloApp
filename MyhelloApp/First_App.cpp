#include "First_App.hpp"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



//std
#include <stdexcept>
#include <cassert>
#include <array>
namespace ikE {

	struct SimplePushConstantData {
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp() {loadModels(),ikeDeviceEngine.createCommandPool(), createPipelinelayout(), recreateSwapChain(), createCommandBuffers(); }
	
	FirstApp::~FirstApp() { vkDestroyPipelineLayout(ikeDeviceEngine.device(), pipelineLayout, nullptr); }

	//here we load the vertices via ikEnginModel
	void FirstApp::loadModels() {
		std::vector<ikEngineModel::Vertex> vertices{
			{{0.0f,-0.5f}, {1.0f,0.0f,0.0f}},
			{{0.5f,0.5f},  {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}

		};
		/*
		  Notice that we accessed the Device via ikeDeviceEngine because in the FirstApp hpp file
		  IkeDeviceEngine ikeDeviceEngine{ ikeWindow } is an instance of IkeDeviceEngine class 
		  that is in ikDeviceEngine.hpp the reason why we can use it as an argument in the ikEngineModel
		  unique_pointer takes ikEngineModel with an instance ikModel ->ikEngineModel loads the Model -> IkeDeviceEngine access the Device and vertices which is in ikEngineModel 
		  
		  */
		//ikModel = std::make_unique<ikEngineModel>(ikeDeviceEngine, vertices);
		 ikModel = std::make_unique<ikE::ikEngineModel>(ikeDeviceEngine, vertices);
	}

	void FirstApp::run() {
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(ikeDeviceEngine.device());
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
		assert(ikSwapChain != nullptr && "Cannot create pipeline before swapChain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");


		PipelineConfigInfo pipelineConfig{};
		ikePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = ikSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		//Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "frag.spv", "vert.spv", pipelineConfig);

		Pipeline = std::make_unique<ikePipeline>(ikeDeviceEngine, "vert.spv", "frag.spv", pipelineConfig);
	}

	/*recreatSwapChain() recreat the swap chain when the window is resized or becomes invalid this function
	  depends on the window surface size, so if the window chainges we need a new swap chain
	  we first assign ike.Window.getExtent() function which is of type Vk2D Extent to extent notice we used auto so that the compiler will
	  deduce the type at run time, it gets the width and height of the window
	  then the while loop where we check that if the swapchain cannot be recreated meaning if it is == to 0
	  ikeWindow.getExtent() is called again 
	  then we call glfwWaitEvents() that tells glfw to put the application to sleep until something happens for example
	  a key is pressed, a mouse is moved or window is resized
	  vkDeviceWaitIdle() with its argument ikeDeviceEngine.device() which is a handle to the logical device that represents our connection
	  to the physical GPU and all queues, command buffers and oprerations that belong to it returns something like a hex number and it is
	  passed to the vkDeviceWaitIdle that tells vulkan to wait before destroying or recreating resources that the GPU might be using, we must wait all operations are complete
	  meaning vkDeviceWaitIdle() that has the ikeDeviceEngine as an argument blocks until all submitted command finishes then it returns control flow back to the program
	  without the vkDeviceWaitIdle destroying the old swap chain could cause GPU crashes
	  then we check if the swapchain exits via if condition then we assign ikEngineSwapChain which is a class or a handle that takes the physical device, the extent 
	  which is the width and height of the window as an argument via std::make_unique a smart pointer which automatically manages the lifetime of the object
	  else if the swap chain already exist we recreat it by passing the old swap chain via std::move()
	  then we update command buffers if needed by checking with the a nested if condition() that if the swapchain imageCount() that holds the size of the swapChainImages which
	  is of type VkImage is not equal to the commandbuffers which is of type vkCommandBuffer it's size because it is wrapped in a std::vector
	  we will free the old commandBuffers and create it again the we go out of the nested if condition
	  because after recreating the swap chain, we usually need a new graphics pipeline  because Pipeline depends on swap chain image format, extent and render pass
	  so if the swapchain changes old Pipeline might not be compatible so we have to call it again via createPipeline()*/
	void FirstApp::recreateSwapChain() {
		auto extent = ikeWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = ikeWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(ikeDeviceEngine.device());
		if (ikSwapChain == nullptr) {
			ikSwapChain = std::make_unique<ikEngineSwapChain>(ikeDeviceEngine, extent);
		}
		else {
			ikSwapChain = std::make_unique<ikEngineSwapChain>(ikeDeviceEngine, extent, std::move(ikSwapChain));
			if (ikSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();

			}
		}
		createPipeline();


	}

	/* createCommandBuffers() creates command buffers object that record all the GPU work we want to execte e.g draws,copies,transitions, e.t.c
	   because in Vulkan we don't issue commands directly to the GPU instead, we record commands into a command buffer and later submit that buffer
	   to a queue which holds the graphics,computer,e.t.c
	   first we resize commandBuffers vector to hold that many vkCommandBuffer handles because we need one command buffer per swapchain image with
	   this we create enough slots for them
	   then we create a VkCommandBufferAllocateInfo struct that tells vulkan how many command buffers i want of which type and from which memory pool
	   .sType set the field telling which type of struct we are using
	   .we set level which is of type enum to VK_COMMAND_BUFFER_LEVEL_PRIMARY meaning these command buffers can be submitted directly to a queue
	   .commandPool is waht will manage the memory of thses buffers
	   .commandBufferCount is the number of buffers to allocate mind you commandBuffers size is uint64 but the std::vector is uint32_t so we need 
	    to use static_cast
		the we check if vkAllocateCommandBuffers which takes the VkDevice which is the logical device, the struct of type VkCommandBuffersAllocateInfo,
		VkcommandBuffer handle which is of type std::vector the reason we use data() to check if it is successful
		*/

	void ikE::FirstApp::createCommandBuffers() {
		commandBuffers.resize(ikSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = ikeDeviceEngine.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(ikeDeviceEngine.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}		
	}
	/*freeCommandBuffers() frees all the Vulkan command buffers previously allocated for rendering .it is typically
	   the cleanup conterpart of a function like createCommandBuffers() inside it we have vkFreeCommandBuffers() and its arguments are
	   ikeDeviceEngine.device() which returns the logical device handle which is the connection to the GPU
	   ikeDeviceEngine.getCommandPool() which returns the VkCommandPool from which this buffers where allocated
	   static_cast<uint32_t>(commandBuffer.size()) which is the number of command buffers to free(vulkan uses uint32_t, not size_t)
	   commandBuffers.data() which is a pointer to the first element of the std::vector<VKCommandBuffer> which is an array handle
	   what happens is that after freeing vulkan simple marks those handles an invalid the memory in the pool is reclaimed for 
	   future allocation but it is not returned to the operating system.
	   After freeing, the std::vector commandBuffers still hold the old handles which is now invalid so we need to call 
	   commandBuffers.clear() that erases all the elements and sets size to 0 to make sure that there is no dangling handles*/
	void FirstApp::freeCommandBuffers() {
		vkFreeCommandBuffers(ikeDeviceEngine.device(), 
			ikeDeviceEngine.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	/*recordCommandBuffer() with and int imageIndex as argument records all GPU commands that draw one frame into a command buffer because each frame in vulkan needs a 
	  command buffer that tells the GPU
	  we start by creating a VKCommandBufferBeginInfo struct that tells Vulkan how you want to start recording commands into command buffer it does not do the recording
	  it only sets the parameters that are needed by vkBeginCommandBuffer()
	  .sType sets the type of struct we are using in vulkan
	  then we can vkBeginCommandBuffer() with an if statement whether it is successfull if not we throw std::runtime error
	  the vkBeginCommand takes the arguments commandBuffers which is an instance of VkCommandBuffer of type std::vector which is a dynamically allocated array  and you 
	  will notice that it has the imageIndex as its index , we also reference the VkCommandBuffersBeginInfo struct via its instance object beginInfo
	  Think of vkBeginCommandBuffer as the press of record on a tape because it is prepares the command buffer to accept recording of commands like(vkCmdDraw,vkCmdcopyBuffer,e.t.c)
	  based on the information profided in the VkCommandBufferBeginInfo struct

	  then we create the RenderPass which tells Vulkan how to use,clear, and store the images your GPU draws into
	  in this case we call create a VkRenderPassBeginInfo struct with
	  .sType sets the field of which type of struct it is in vulkan
	  .renderPass which tells vulkan which renderpass to use(color + depth attachment,subpasses,e.t.c)
	  .framebuffer tells vulkan the specific image to render into
	  .renderArea tells vulkan the area of the framebuffer we will draw to i.e the whole screen
	   the renderArea.offset defines the top-left corner(x,y) of the rectangle within the framebuffer 
	   where rendering will occur so {0,0} means to start rendering from the top-left corner of the 
	   framebuffer(no offset)
	   the renderArea.extent defines the width and height(in pixels) of the rendering region
	  Note that renderPass and framebuffer define the output target of rendering

	  then we create a fixed-sized array of 2 elements of type VkClearValue which is a union
	  and 2 represent the two members of this union type that is depth and stencil
	  we access the first array element which .color and fills it with RGBA
	  we access the secomd array element which is .depthstencil and fills it with
	  the depth meaning the farthest depth which clears everything
	  the stencil 0 resets the stencil buffer to 0
	  .clearValueCount tells vulkan how many attachments to clear in this case it is 2
	  .pClearValue gives Vulkan a pointer to the array of clear values
	  then we record to command buffer to begin the renderPass that takes 2 arguments

	  then we Define the viewport and scissor rectangle
	  the viewport defines how normalize device cordinates (-1....1) map to framebuffer pixels
	  the scissor defines the rectangle area of the framebuffer that can be written to
	  both are GPU pipelines states that control rasterization
	  we first create the viewport struct which is of type VkViewport
	  .x and .y defines the top-left corner(in pixel cordinates) of the rendering area
	  inside the framebuffer, remember that in vulkan the y axis in screen space point down
	  by default unlike opengl where it is up, you can flip it if you want by inverting it
	  .width and . height defines how wide and tall the viewport is which is normally the
	  same size as the swapchain extent(the window size in pixel)
	  .minDepth and maxDepth defines the range of depth values(Z-axis) that map the depth buffer
	   0.0 is the nearest, 1.0 is the farthest
	   then we call the scissor which is of type VkRect2D that defines a cut-out area of the framebuffer
	   where pixels can actually be written, Anything outside is discarded(not even rasterized)
	   it is a pixel-space bounding box, not a cordinate transform
	   0,0 is the offset(x,y) and getSwapChainExtent() it the width,height
	   then we call vkCmdSetViewport that takes the (commandBuffers, firstViewport,viewportCount and the vkviewport)
	   as an argument. it tells vulkan to use this viewport configuration for the next draw commands
	   then we call vkCmdSetScissor that tells vulkan to only draw inside the rectangular area

	   then the actual rendering commands go in to 
	   Pipeline->bind() which Records a command to bind the pipeline object(shaders,fixed-function states, e.t.c)
	   ikModel->bind() Bindes the vertex and index buffers( it tells GPU where the geometry data lives in memory)
	   ikMode->draw() issues a draw call (e.g, vkCmdDrawIndexed) that schedules GPU vertex processing, rasterization, and fragment shading

	   then we end the render pass by calling vkCmdEndRenderPass() which tell the GPU that we are done writing commands for this render pass
	   then we end the command buffer recording using an if statement to check if vkEndCommandBuffer() is successful if not throw std::runtime error

	*/
	void FirstApp::recordCommandBuffer(int imageIndex) {

		static int frame = 0;
		frame = (frame + 1) % 1000;


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffers !");

		}
		//this Describe the render pass you want to begin
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = ikSwapChain->getRenderPass();
		renderPassInfo.framebuffer = ikSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = ikSwapChain->getSwapChainExtent(); 

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f,0.01f,0.01f,1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 }; // instead of 1.0f,0.0f
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		//we record to the command buffer to begin the renderPass
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(ikSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(ikSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, ikSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
		   
		//Bind the graphics pipeline and draw the model
		Pipeline->bind(commandBuffers[imageIndex]);
		ikModel->bind(commandBuffers[imageIndex]);

		for (int j = 0; j < 4; j++) {
			SimplePushConstantData push{};
			push.offset = { -0.05f + frame * 0.002 , -0.4f + j * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + 0.2f * j };

			vkCmdPushConstants(commandBuffers[imageIndex],
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
		  		sizeof(SimplePushConstantData),
				&push
			);
           ikModel->draw(commandBuffers[imageIndex]);
		}
		

		// End the render pass and CommandBuffer
		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	/*drawFrame() is a function that helps to draw images to the screen and it does that using 4 steps
	   imageIndex of type uint32_t tells which image the swapchain to use for this frame
	   we call acquireNextImage with a reference operator to imageIndex because the acquireNextImage argument has a pointer variable
	   via a unique_ptr which is ikSwapChain and stores the return value inside result
	   we check if result does not fit the swapChain window by if its equal to VK_ERROR_OUT_OF_DATE_KHR which is a member of the enum of type result
	   then we recreate the swap chain and return

	   then we also check for other type of errors that is if the result is not successful and the swapchain works but it is not optimal we throw std::error

	   then we call recordCommandBuffer with the imageIndex as an argument
	   then we submit the command buffer for execution by assing submitCommandBuffers which has two pointers as arguements to result
	   then we handle post-submit swapchain state check if the window was resized if not 
	   we call the resetWindowResizeFlag that is false the we call recreateSwapchain() again amd return the fuction flow


	   */

	void ikE::FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = ikSwapChain->acquireNextImage(&imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image !");
		}

		recordCommandBuffer(imageIndex);
		result = ikSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ikeWindow.wasWindowResized()) {
			ikeWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image !");
		}
	}






}//namespace ikE
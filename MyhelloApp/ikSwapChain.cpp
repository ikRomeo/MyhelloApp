#include "ikSwapChain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace ikE {
	ikEngineSwapChain::ikEngineSwapChain(IkeDeviceEngine& deviceref, VkExtent2D extent) 
		                                  : device{ deviceref }, windowExtent{ extent } {
		init();
	
	}
	ikEngineSwapChain::ikEngineSwapChain(IkeDeviceEngine& deviceref, VkExtent2D extent, std::shared_ptr<ikEngineSwapChain> previous)
		: device{ deviceref }, windowExtent{ extent }, oldSwapChain{ previous } {
		init();

		// clean up old swap chain since it's no longer needed
		oldSwapChain = nullptr;

	}


	void ikEngineSwapChain::init() {

		createSwapChain();
		createImageViews();
		createRenderPass();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
        }


	ikEngineSwapChain::~ikEngineSwapChain() {
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device.device(), imageView, nullptr);

		}
		swapChainImageViews.clear();

		if (swapChain != nullptr) {
			vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
			swapChain = nullptr;
		}

		for (int i = 0; i < depthImages.size(); i++) {
			vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
			vkDestroyImage(device.device(), depthImages[i], nullptr);
			vkFreeMemory(device.device(), depthImageMemorys[i], nullptr);
		}

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
		}
		
		vkDestroyRenderPass(device.device(), renderPass, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device.device(), inFlightFences[i], nullptr);
		}
	}

	//when CPU ask Vulkan which swapchain image is available for rendering
	// it does that through acquireNextImage() so we call it with a uint32_t type as an argument
	// inside the function we call vkWaitForFences which helps to track when the GPU has finished work
	// submitted earlier if its not it blocks the CPU thread until one or more GPU fences are signaled
	// which prevents the CPU for overwritting resources still in use by the GPU
	// inside the vkWaitForFences we have the device.device() which is vulkan logical device
	// 1 which is the number of fences to wait on
	// &inFlightFences[currentFrame] pointer to the fence for the current frame
	// Vk_TRUE means wait for all fences in this case 1
	// std::numberic_limits<uint64_t>::max() is the time in nanoseconds because uint_64 can hold multiple digits
	// vkAcquireNextImageKHR is the function the CPU uses to get the next image from the swap chain
	// it simply ask vulkan for the next index of the next available swap chain image to render to
	// device.device() is the logical device handle
	// swapChanin the swap chain we created earlier
	// std::numberic_limits<uint64_t>::max() time out in nanosecond
	// imageAvailableSemaphores[currentFrame] semaphore that will be signaled when the image is 
	// ready for rendering and imageAvailableSempahore is an instance of std::vector<VkSemaphore>
	// VK_NULL_HANDLE because we called the vkWaitForFences() earlier the parameter is not used
	// and the imageIndex which is the argument of our acquireNextImage()
	// the we return the result
	
	// Note that this code needs to be refactored because the validation layer is warning us
	// about the acquireswapchain per frame instead of per image
	VkResult ikEngineSwapChain::acquireNextImage(uint32_t* imageIndex) {
		vkWaitForFences(device.device(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	
		VkResult result = vkAcquireNextImageKHR(device.device(),
			swapChain, std::numeric_limits<uint64_t>::max(),
			imageAvailableSemaphores[currentFrame],
			VK_NULL_HANDLE,
			imageIndex);
		
		return result;
	}
	








	// submitCommandBuffers() with three parameter list is responsible for 
	// submitting a recorded command buffer to the GPU for rendering
	// Handling sychronization through fences and semaphores
	// presenting the rendered image to the swap chain
	// Managing frames in fight for pipeline rendering
	// inside the function we start with an if statement
	// the imagesInFlight[] is an array which is an object of std::vector<VkFence> handle
	//  has imageIndex pointer as an integer index of the array and it is compared to VK_NULL_HANDLE if true
	//  it tracks which Fence is currently using this swap chain image
	// inside the if statement we call vkWaitForFences() if the image is still being
	// used by the GPU (fence not signaled), CPU waits indefinitely until the GPU finishes(UINT64_MAX)
	// then the image is assigned the current frame's fence so we know it's in use
	// by assigning inFlightFences array to imageInFlight notice that both are VKFence object
	// we then create a struct to submint the commands called VkSubmitInfo
	// .sType it required for vulkan to know which type of struct it is dealing with
	//  waitSemaphores[] is an array of type VkSemaphore which is a sync between 
	//  GPU to GPU or GPU presentation Engine it's purpose is to regulate ordering
	//  operations across queues/pipelines making sure things do not start too early
	//  and avoiding race conditions in GPU execution
	//  so we assigne imageAvailableSemaphores which is also a VkSemaphore object with currentFrame to it
	//  waitStages[] is also an array of pipeline that is assigned and of type enum VkPipelineStageFlags it's
	//  index is VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT which is final pixel blending and writing to framebuffer
	//  meaning it  will ensure that the GPU waits before writing to framebuffer
	//  waitSemaphoreCount is of type uint32_t lets vulkan know how many semaphores it wants the batch to wait on
	//  before starting execution in this case is 1
	//  .pWaikSemaphores also an array of VkSemaphore handle is used to signale each semaphore
	//   before work begins
	//  .pWaitDstStageMask is also of type VkPipelineStageFlags which is an enum and has the same
	//   length as waitSemaphoreCount tis tells the vulkan at which pipeline stage the semaphores
	//   wait should take effect
	//  .commandBufferCount = 1 Number of command buffers to submit
	//  .pCommandBuffers array of command buffers the buffers contain the recorded GPU commands
	//   like draw calls,dispatch,barriers e.t.c in this case it is the rendering command
	//  signalSemaphore{] then we have to signal the GPU with renderFinishedSemaphores with the currentFrame as index
	//  that the command buffer has finished execution so that the CPU can present it safely
	//  notice that because the commandBufferCount is 1 the signalSemaphore count is also 1
	//  .pSignalSemaphores which is also ov type VkSemaphore handle is assigned the signalSemaphores array
	//  vkResetFences takes one more fences and resets them from signaled to unsignaled so it can be reused
	//  here it takes in the logical device(), the fenceCount which is 1 and VkFence handle
	//  then we submit the commands to the graphics pipeline via vkQueueSubmit
	//  vkQueueSubmit takes the device(), the number of batches of work we are submitting
	//  and the inFlightFences[currentFrame] which tells vulkan to associate fence with commands
	//  we just submitted.When the GPU finishes this submission, signal the fence to continue
	//  processing the next submission 
	//  remember that fence is CPU and semaphores is GPU synchronization
	//  to present the image into swapchain for rendering we need to create a VkpresentKHR struct
	//  we create the zero initialized struct
	//  .sType which is a must in vulkan before filling up any struct so vulkan know which type of struct it is
	//  .waitSemaphores there is only one semaphore to wait on
	//  .pWaitSemaphores wait until rendering is done
	//  .swapchainCount in our case is just 1
	//  .pSwapchain is a pointer to an array of VKSwapchainKHR handles 
	//  .pImageIndices image acquired earlier
	//   now we assign the vkQueuePresentKHR to result the arguments is 
	//   we get the vulkan queue via device.presentQueue(), and a pointer to a VkPresentInfoKHR struct
	//   that describes which semaphore we must wait on before presenting, which swapchain we are presenting to
	//   which image index of the swapchain we want to present and where to store it if needed
	//  then we update the frame index for the next iteration of the render loop via
	//  MAX_FRAMES_IN_FLIGHT by shifting it 1 place and wrap in it around via modulo because it is a circular buffer
	VkResult ikEngineSwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
		if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		imagesInFlight[*imageIndex] = inFlightFences[currentFrame];
		//prepare the semaphores
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// wait semaphores
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//command Buffers
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;
		//signalSemaphores
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
		if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffers!");

		}
		// submit the presentation request into the swapchain and tells the presentation engine or display system
		// to show it on screen
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		return result;

	}

	// createSwapChain is an infastructure that holds the framebuffers we render to before 
	// visualizing it on the screen 
	// we start by quering the swapchain via getSwapChainSupport() which is a function in ikDeviceEngine
	// it is used to see the hardware and driver support and it is of type SwapChainSupportDetails 
	// which is a struct
	// we then chooseSwapChain properties via SurfaceFormat which choses the best color and color space
	// we choose how the images are swapped to the screen i.e vsync or triple buffering via PresentMode
	// we also choose the resolution of the swap chain images which usually matches the window size
	// via SwapExtent
	// next we descide how many images we want to render because vulkan requires at least a minimumImageCount images
	// for a swapchain we use one more than the minimum to allow triple buffering that is the reason
	// we use the if loop condition to get it
	// then we fill the VkSwapchainCreatenfoKHR struct by accessing the .surface which is a member of the struct
	// we set the Image Properties through
	// .minImageCount number of images
	// .imageFormat and ImageColorSpace the format of the images
	// .imageArrayLayers which is 1 for normal images more than 1 if for stereoscopic 3D
	// .imageUsage what the images will be used for (color attachment = rendering target)
	// we then handle queue families by
	// first instancing QueueFamilyIndices struct and assigning the findPhysicalQueueFamilies()
	// we create an array of type uint32_t of 2 indices the graphics and present family which is used as ID's
	// then we access the imageSharingMode with the help of an if condition
	// we will  check if the graphicsFamily is not equal to presentFamily
	// we will use 2 indexcount and make the sharing mode CONCURRENT if it is equal we will use
	// the EXCLUSIVE and the pQueueFamilyIndices will be nullptr instead of queueFamilyIndices
	// we will then set the 
	// .preTransform a member of VkSwapchainCreateInfo struct via currentTransform a member in VkSurfaceCapability struct
	//  which tell the vulkan how the images are rotated or
	//  flipped before presenting (usually IDENTITY)
	// .compositeAlpha is how the alpha channel of the image is handled with the window system and here we make
	//  OPAQUE meaning we ignore transperancy for now
	// .presentMode chosen mode for presenting(vsync, mailbox e.t.c)
	// .clipped we made it true meaning pixels covered by another window don't need to be drawn
	//  .oldSwapchain here is where when we want to recreate the swapchain i.e resizing the screen window
	//   because this is the first creation of the swapchain we will assign VK_NULL_HANDLE to it
	// then we will call vkCreateSwapchain() and give it the getter device(), the createInfo struct,
	// the allocationcallbacks pointer which we will make nullptr and vkswapchainKhr handle that is instanced by swapchain
	void ikEngineSwapChain::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = device.surface();
		
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };


		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;     //Optional
			createInfo.pQueueFamilyIndices = nullptr; //Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

		if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chains!");
		}

		/*we only specified a minimum number of images in the swap chain, so the implentation is allowed
		  to create a swap chain with more. That's why we'll first query the final number of images with
		  vkGetSwapChainImagesKHR, then resize the container and finally call it again to retrieve the handles*/
		vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	// createImageViews creates one VkImageView for each VkImage in the swap chain
	// A VkImageView tells Vulkan how to look at an image(format,aspect,mip level, layers e,t,c)
	// without an image view, you can not use an image in a framebuffer
	// we start by resizing the <VkImageView> dynamic array of std::vector
	// with an argument which is another dynamic array VkImage that is instanced by swapChainImages
	// the reason for this is so that it has the same number of elements as the swapChainImages
	// we resized earlier in createswapchain() notice that we are getting the number or swapchain
	// images from <VkImage>
    // then we loop through each swapchain image index
	// the index i will be used to get the corresponding VKImage ,store the created VKImageView
	// inside the for-loop we proceed to create a zero initialized to struct and fill it up
	// it contains all the parameters needed to create an image view meaning a view into an image
	// .sType required by vulkan to identify the type of struct we are dealing with
	// .image this is the VkImage handle for the current swap chain image
	//  the  image view will be linked to this image
	// .viewType specifies the type of image and it is TYPE_2D for now
	// .format is the pixel format i.e (VK_FORMAT_B8G8R8A8_SRGB)
	//  this must match the format when creating the swap chain images
	// .subresourceRange.aspectMask tells the vulkan which part of the image
	//  the view will access the VK_IMAGE_ASPECT_COLOR_BIT color data(swap chain images are for color)
	//  for depth/stencil buffers, you'd use VK_IMAGE_ASPECT_DEPTH_BIT OR VK_IMAGE_ASPECT_STENCIL_BIT
	// .subresourceRange mipmaps are different resolutions of the same texture
	//  and we are only using the base mip level(level 0) and only 1 mip level
	// .baseArrayLayer = 0 this specifies which array layers the view should access
	// .layerCount = 1 for 2D images, there usually only 1 layer
	//  then the if statement to call vkCreateImageview with the logical device that owns the image
	//  a pointer to VKImageViewCreateInfo, pallocator which is null and VkImageView pointer which
	//  we referenced with swapChainImageView at index i which is the handle we will bind in framebuffer,
	//  descriptors e.t.c
	void ikEngineSwapChain::createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapChainImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = swapChainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view!");
			}
		}

	}
	// A renderPass is like a blueprint for how rendering into framebuffers will happen
	// so The createRenderPass() describes the attachments(color/depth images) how they are used(loads/store operations)
	// the subpasses(sections of the rendering pipeline), synchronization between subpasses
	// we first start by calling a zero initialzed struct notice that we did not use the 
	// sType in VkAttachmentDescription because there is no word as Create in it name
	// the VkAttachmentDescription is for the depth Attachment setup
	// .format has findDepthFormat() assigned to it which is responsible in picking a supported format
	// .sample has Vk_sample_count_1_bit meaning we are not using multisample anti aliasing(MSAA)
	// instead we using single-sample per pixel
	// .loadOp this defines what happens to the color/depth contents
	//  of the attachment at the start of the render pass the depth buffer is cleared at the start of the render pass
	// .storeOp defines what happens to the color/depth contents of the attachment at the end of the render pass
	//  the VK_ATTACHMENT_STORE_OP_DONT_CARE means we do not save the dpeth after the render pass(only needed
	//  for depth testing, not post-processing)
	// .stencilLoadOp same as loadOp,but specifically for the stencil aspect(if using a depth stencil attachment)
	// .stencilStoreOp sam as storeOp, but for stencil aspect
	// .initialLayout layout of the image before the render pass begins it tells vulkan what state the image
	//  is in when the pass starts
	// .finalLayout lyout of the image after the render pass ends VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	//	Tells vulkan that if the contiuing as depth
	// then we create another zero initalized struct VkAttachmentReference struct which is used to tell vulkan
	// which image attachments are used in which subpasses and how they should be accessed
	// .attachment this specifies the index of the attachment in the VkRenderPassCreateInfo::pAttachment array
	//  which is 1 in our case meaning the second attachment
	// .layout this specifies the layout the attachment must be in during the subpass which is
	// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL which is an imput attchment in a shader
	// then we call another zero initialized struct VkAttachmentDescription for the colorAttachment setup
	// .format will be assign getSwapChainImageFormat() which is of type VkFormat
	// .samples number of samples per pixel in our case it means no MSAA
	// .loadOp defines what happens to the attachment contents at the start of the render pass
	// .storeOp defines what happens to the attachment contents at the end of the render pass
	// .stencilLoadOp same as loadOp but applies only to the stencil aspect
	// .stencilStoreOp same as the storeOp but applies only to the stncil aspect
	// .initialLayout the layout the image will be in before the reder pass starts
	// .finalLayout meaning it end in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR so the swapchain can present it
	// we then initailize another zero struct VkAttachmentReference
	// .attachment refers to the first attachment in the array at index 0
	// .layout will be used as the color target in the subpass
	
	// we then initialize another zero initialize struct for the subpass Description
	// subpass is a phase of rendering that can read/write attachments(like color, depth, or input attachments)
	// .pipelineBindPoint it indicates the type of pipeline that will be used in this subpass
	//  VK_PIPELINE_BIND_GRAPHICS is for standard graphics pipeline(not compute)
	// .colorAttachmentCount is the number of input attachments used in this subpass in this case is 1
	// .pcolorattachment a pointer to an array of VkAttachmentReference for color attachments
	//  each reference tells Vulkan which attachment ot write to and in which layout
	// .pDepthStencilAttachment a pointer to a single VkAttachmentReference for the depth/stencil buffer
	//  this is used for depth test, stencil test, and depth writes
	// we now create another zero initialized struct VkSubpassDependency
	// VkSupbassDependency this struct defines dependencies between subpasses, ensuring proper ordering
	// and memory visibility.it controls when one subpass can start using resources written by another
	// .srcSubpass index of the source subpass the VK_SUBPASS_EXTERNAL refers to operations outside the
	//  render pass, like before the first subpass or after the last
	//  this is the subpass that produces data or has side effects that the destination subpass depends on
	// .srcAccessMask pipeline stage in the destination subpass that must wait for the source it
	//  tells vulkan where to block execution until the source is done in the zeroth index
	// .srcStageMask pipeline stages in the source subpass that must complete before the dependency
	//  it defines which stage of the pipeline produces the data we care about
	// .dstStageMask is the pipeline stages in the destination subpass that must wait for the source
	//  it tell vulkan where to block execution until the source is done
	// .dstAccessMaskt tells vulkan what type of memory access the destination subpass needs to wait
	//  for before starting
	//  we will now create an array via std::array with 2 index that will be passed to the VKRenderPassCreateInfo struct
	//  we will now create a zero initialized struct of VkRenderPassCreateInfo struct
	// .sType sets the type of struct required by vulkan
	// .attachmentCount sets the number of attachments in the render pass via attachment.size() which is 2
	//  then we ensure the type matches Vulkan uint32_t by casting it notice that size() is not a primitive type but a
	//  a numeric value of uint64_t and we are casting it to uint32_t which is safe via static_cast
	// .Attachments passes a pointer to the array attachments. Vulkan will use this to know which images
	//  exist in the render pass and how they behave
	// .subpassCount the render pass has one subpass most simple pipelines have a single subpass for drawing
	// .pSubpasses it is assigned &subpass which is a VkSubpassDescription struct and tells vulkan how this
	//  subpass uses the attachments(color,depth,input attachments, e.t.c)
	// then we call vkCreateRenderPass() which has the logical device,VkRenderPasCreateInfo struct,
	// pallocator which we made a nullptr and VkRenderPass handle with a if statement
	void ikEngineSwapChain::createRenderPass() {
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = getSwapChainImageFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");

		}

	}

	// a framebuffer is a collection of attachments(color,depth,etc) that a render pass will render into
	// so createFramebufffers() is a functon that helps create framebuffers for every image in the swapchain
	// swapChainFramebuffers is an std::vector<VkFramebuffers> and imageCount() returns the number of images
	// in the swapchain because each swapchain image will have corresponding framebuffer, so we resize the
	// vector to hold all of them
	// we iterate over each swapchain image because each image needs a framebuffer to attach its color and depth views
	// inside the for loop the vkImageView has 2 attachments which is
	// swapChainImageView[i] which is the color attachment (the swapchain image that will be presented)
	// depthImageViews[i] which is the depth attachment(used for depth testing)
	// with swapChainExtent which is of type VkExtent2D and assigned getSwapChainExtent() which
	// is also of type VkExtent2D we will be able to return the width and height of the swapchain images
	// we will then create VkFramebufferCreateInfo which is a zero initialized struct
	// .sType identifies the struct type
	// .renderPass will be assign renderPass which is of type VkRenderPass handle
	// .attachmentCount is the number of attachment which is 2 in this case (color and depth)
	//  std::array<VkImageView,2> is a vector and the attachment.size() returns a size_t type
	//  which is 64 bits on 64 bit systems , attachmentCount is a uint32_t type meaning a 32bit unsigned
	//  we will need a static cast it meaning a wider range to a narrower range else the compiler will
	//  throw an error
	//  .width we assign the width which is of type VkExtent2D
	//  .height we assign the height which is of type VkExtent2D
	//  .layers is the number of layers which is 1 fo a standard 2D images
	//  
	void ikEngineSwapChain::createFramebuffers() {
		swapChainFramebuffers.resize(imageCount());
		for (size_t i = 0; i < imageCount(); i++) {
			std::array<VkImageView, 2>attachments = { swapChainImageViews[i], depthImageViews[i] };

			VkExtent2D swapChainExtent = getSwapChainExtent();
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	// the swapchain images only provide color attachments(the image shown on screen)
	// To render correctly in 3D,we will need a depth buffer to store per-pixel depth values,
	// so the GPU can descide which fragments are in front or behind
	// the createDepthResources() creates one depth image + memory+image view for each swapchain image
	// we start by assigning findDepthFormat() of type VkFormat which is an enum and it picks a supported GPU format
	// (commonly VK_FORMAT_D32_FLOAT or VK_FORMAT_D24_UNORM_S8_UINT) its job is to find suitable depth buffer format
	// for the swap chain/framebuffer
	// getSwapChainExtent wich is of type VkExtent2D matches the swapchain resolution, so the depth buffer is the same
	// size as the color image
	// depthImages of type VkImage handle allocates array to hold a depth image
	// depthImageMemory of type VkDeviceMemory handle is also an array that holds memory
	// depthImageView of type VkImageViews handle is also an array that holds image view per swapchain image
	// Each swapchain image need its own depth attachment, because multiple frames can be in flight
	// we now create the Depth Image by creating a zero initialized struct of VkImageCreateInfo
	// .sType is an enum required by vulkan to know which struct we are dealing with
	// .imageType is the Dimensionality of the image
	// .extent.width is the width of the image
	// .extent.height is the height of the image
	// .extent.depth in this case is 1 because it is a 2D image
	// .mipLevels is the number of mipmap levels in the case because it is depth it is 1 
	//  if it was for textures we might use multiple mipmaps    
	// .arrayLayers is the number of layers in the image array for depth buffer is usually 1
	// .format specifies how pixel data is stored
	// .tiling which is memory layout preference here we are using optimal layout for GPU
	// .usage Bitmask of how the image will be used
	// .samples here we use VK_SAMPLE_COUNT_1_BIT for non-MSAA which is multisampling
	// .Controls ownership across queue families SHARING_MODE_EXCLUSIVE means owned by one queue family at a time
	// .bitmask of VkImageCreateFlagBits which is 0 for depth images
	// then we call createImageWithInfo which stores handle in depthImage[i] which is of type VkDeviceMemory handle
	// and its memory in depthImageMemorys[i] which is of type VkImageView handle
	// it creates the VkImage and allocates GPU memory via VkDeviceMemory , Device-local memory is fast GPU memory
	// which is not visible to CPU and perfect for depth
	// the next zero initialized struct is VkImageViewCreateInfo which describes how to create a view into an image
	// because images in Vulkan are raw memory witn no inherent interpretation An Image view tells vulkan how to 
	// access that image, its format,mipmap levels, array layers and component mapping
	// .sType is an enum that is required by vulkan to let it know which type of struct it is dealing with
	// .image is the handle to the image you're creating the view for in this case depthImage array of type
	//  VkImage handle at index i 
	// .viewType is how we want to view the image in this case it is VK_IMAGE_VIEW_TYPE_2D
	// .format which tell vulkan the pixel format for the view and it is of type Vkformat
	// the subresourceRange describes which part of the image the view will access
	// .subresourceRange.aspectMask tells vulkan which aspect of image of an image you want to work  with
	// in this case VK_IMAGE_ASPECT_DEPTH_BIT this selects the depth aspect of an image
	// .subresourceRange.baseMipLevel tells vulkan the first mipmap level to use in this case it is 0
	// .subresourceRange.levelCount is the number of mip levels in the case 1
	// .subresourceRange.baseArrayLayer First array layer to use in this case 0
	// .layerCount is the number of array layers in this case is 1
	// then we call vkCreateImageView with an if statement with arguments device.device() which is the logical device,
	// address to Viewinfo which is of type vkCreateImageView struct, pAllocator which is a pointer and we leave it
	// as a nullptr,depthImageViews of type VkImageView handle and check if it is succesfull else throw std error
	void ikEngineSwapChain::createDepthResources() {
		VkFormat depthFormat = findDepthFormat();
		VkExtent2D swapChainExtent = getSwapChainExtent();

		depthImages.resize(imageCount());
		depthImageMemorys.resize(imageCount());
		depthImageViews.resize(imageCount());

		for(int i = 0; i < depthImages.size(); i++){ 
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = swapChainExtent.width;
			imageInfo.extent.height = swapChainExtent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = depthFormat;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = depthImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = depthFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create texture image view! ");
			}
		
		}
	}
	// because vukan rendering is highly parallel and asynchrounous we need a sycnhronization primitives to coordinate
	// i.e semaphores which is GPU to GPU operations(wait uintil the image is ready before rendering, wait until rendering
	// is done before presenting it) fences which is GPU to CPU operation(CPU waits until GPU has finished a frame before 
	// reusing resources)
	// in createSyncObjects() we use binary semaphores which is default in vulkan is responsible for
	// creating all the primitives the renderer will need for managing frames in flight because in vulkan
	// we must explicitly create semaphores and fences before we can use them
	// the createSyncObjects() crates 1 semaphore which is imageAvailableSemaphore per frame
	// 1 semaphore which is renderFinishedSemaphores per frame and 1 fence which is inFlighFence per frame
	// imageAvailableSemaphores which is of type VkSemaphore handle is resized and its argument is 
	void ikEngineSwapChain::createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame !");
			}
		}
	}
	VkSurfaceFormatKHR ikEngineSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR ikEngineSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		//MailBox(low letency,no tearing)
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Present mode: Mailbox" << std::endl;
				return availablePresentMode;
			}
		}
		// Then try IMMEDIATE (low latency, may tear)
		for (const auto& mode : availablePresentModes) {
			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				std::cout << "Present mode: Immediate (may tear)" << std::endl;
				return mode;
			}
		}



		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ikEngineSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;

		}else {
			VkExtent2D actualExtent = windowExtent;
			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));
			//actualExtent.width = std::max(capabilities.minImageExtent.width, actualExtent.width,std::min(capabilities.maxImageExtent.width,actualExtent.width));
			//actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height,actualExtent.height));

			return actualExtent;
		}
	}

	VkFormat ikEngineSwapChain::findDepthFormat() {
		return device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT,VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}







}//namespace
#include "ikDeviceEngine.hpp"

//std headers
//#include <stdexcept>
#include <iostream>
#include <set>
#include <unordered_set>
#include <cstring>


namespace ikE {


	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
	//class member function
	//constructor
	IkeDeviceEngine::IkeDeviceEngine(IkeWindow& window) : window(window) {
		createInstance();
		setupDebugMessenger();
		//
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}
	//destructor
	IkeDeviceEngine::~IkeDeviceEngine() {
		vkDestroyDevice(device_, nullptr);
		//
		if (enableValidationlayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		//
		vkDestroySurfaceKHR(instance, surface_, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwTerminate();
	}


	void IkeDeviceEngine::createInstance() {
		if (enableValidationlayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "BronixEngine App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
		createInfo.ppEnabledExtensionNames = extensions.data();



		if (enableValidationlayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
			createInfo.ppEnabledLayerNames = validationlayers.data();
		}
		else {

			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance");

		}
		hasGlfwRequiredInstanceExtensions();
	}

	void IkeDeviceEngine::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		//if there is no devices with vulkan support there is no point going further
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPU's with Vulkan Support!");

		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// check if the device meets the requirements
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		std::cout << "physcial device :" << properties.deviceName << std::endl;

	}


	// createLogicalDevice
	void IkeDeviceEngine::createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		//VK_KHR_swapchain enabling
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		//Might not be necessary anymore but  we will still write it
		if (enableValidationlayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
			createInfo.ppEnabledLayerNames = validationlayers.data();

		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device");
		}
		//
		vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);

	}

	
	void IkeDeviceEngine::createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool");
		}
	}

	//Note:this needs explanation
	void IkeDeviceEngine::createSurface() { window.createWindowSurface(instance, &surface_); };


	
	
	
	
	
	bool IkeDeviceEngine::isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

		}
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	void IkeDeviceEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; //optional

	}


	//still need to be populated
	void IkeDeviceEngine::setupDebugMessenger() {
		if (!enableValidationlayers) return;

		//we call the struct and the function
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);
		// we check if it has the instance, the struct VkDebugUtilsMessengerCreateInfoEXT,
		// the struct *pAllocator set to null,the VkDebugUtilsMessengerEXT Handle
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
			&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool IkeDeviceEngine::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//we iterate to find the layers
		for (const char* layerName : validationlayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerName) {
				return false;
			}
		}
		return true;
	}

	

	std::vector<const char*> IkeDeviceEngine::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		// Create a vector from GLFW extensions
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// Add debug extension if validation layers are enabled
		if (enableValidationlayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		}
		return extensions;
	}


	void IkeDeviceEngine::hasGlfwRequiredInstanceExtensions() {
		//query available extensions for extension list and stores the number in extensionCount
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		//retrieve extensin properties
		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		//stores availabe extensions in unordered_sets
		std::cout << " yeah available extensions:\n";
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
			available.insert(extension.extensionName);
		}
		// we verify required extensions if not available throws an error
		std::cout << "Required extensions:\n";
		auto requiredExtensions = getRequiredExtensions();
		for (const auto& required : requiredExtensions) {
			std::cout << "\t" << required << std::endl;
			if (available.find(required) == available.end()) {
				throw std::runtime_error("Missing required glfw extension");
			}
		}

	}


	bool IkeDeviceEngine::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());
		
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		
		return requiredExtensions.empty();
	}


	QueueFamilyIndices IkeDeviceEngine::findQueueFamilies(VkPhysicalDevice device) {

		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
				indices.graphicsFamilyHasValue = true;
			}
			//Note:this need explanation
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport) {
				indices.presentFamily = i;
				indices.presentFamilyHasValue = true;
			}
			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;

	}


	SwapChainSupportDetails IkeDeviceEngine::querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
		}
		return details;
	}


	


	VkFormat IkeDeviceEngine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format");
	}
   
	

	uint32_t IkeDeviceEngine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type !");
	}





	void IkeDeviceEngine::createBuffer(
		VkDeviceSize size,                   //the size in bytes of the buffer to create
		VkBufferUsageFlags usage,            // how the buffer will be used e.g vertex buffer, uniform buffer
		VkMemoryPropertyFlags properties,    // Specifies memory requirements e.g devic-local,host-visible
		VkBuffer& buffer,                    // Output parameter, the created vulkan handle
		VkDeviceMemory& bufferMemory) {      // Output parameter, the allocated device memory backing the buffer

		VkBufferCreateInfo bufferInfo{};                            // an initialized struct
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;    // Required for Vulkan structures to identify themself
		bufferInfo.size = size;                                     // Sets the buffer size (how much memory it will hold)
		bufferInfo.usage = usage;                                   // tells vulkan what the buffer is used for e.g VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;         // means only one queue family e.g graphics will use this buffer at a time

		if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");  // if the vkCreateBuffer fails it throws a runtime error
		}

		VkMemoryRequirements memRequirements;   
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements); // we then query the memory requirments

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;           // the size of the memory blocks you want to allocate on the device
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);  // index ino the physical devices memory types array

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device_, buffer, bufferMemory, 0);  // links the memory block to the buffer
	}




	VkCommandBuffer IkeDeviceEngine::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};                             // initializes a struct with zero/default values
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;   //  required for vulkan structures to identify them self
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;                                   // the handle we need to begin recording
		vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);   //

		VkCommandBufferBeginInfo beginInfo{};                           // initializes a struct with zero/default values
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;  // Required for vulkan structures to identify them self
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // Specifies that the command buffer will be used for a single submission

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void IkeDeviceEngine::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue_);

		vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);

	}
	
	void IkeDeviceEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; //Optional
		copyRegion.dstOffset = 0; //Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}


	void IkeDeviceEngine::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer,buffer,image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endSingleTimeCommands(commandBuffer);
	}


	void IkeDeviceEngine::createImageWithInfo(
		const VkImageCreateInfo& imageInfo,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory) {

		if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device_, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory !");
		}

		if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS) {
			throw std::runtime_error("failed to bind image memory!");
		}
	}





} //namespace




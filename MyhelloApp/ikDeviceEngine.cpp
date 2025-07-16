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
		//uint32_t glfwExtensionCount = 0;
		//const char** glfwExtensions;
		//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


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
	// this needs explanation
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


	/*The function isDeviceSuitable verifies whether a Vulkan GPU (physical device) meets the necessary requirements for use. Specifically, it ensures:
	   1. The device has queue families for rendering and presenting.
	   2. The required Vulkan extensions are available.
	   3. The swap chain is adequate for rendering.
	   4. The device supports anisotropic filtering.
	   If all these conditions are satisfied, the function returns true, meaning the device is suitable for Vulkan rendering. Otherwise, it returns false.*/
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
	//call getRequiredExtensions to retrieve glfw required extensions
	// glfw usually requires Vk_KHR_surface to create vulkan surface
	// and platform-specific surface extension depending on the os
	// Vk_KHR_win32_surface for windows
	// VK_KHR_xcb_surface for linux
	// VK_KHR_wayland_surface for wayland
	// other optional extensions like VK_EXT_debug_utils for validation layers
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


	//checks whether all the required Glfw instance extensions are available on the system before proceeding with vulkan initialization
	/* 1. Queries the number of available vulkan instance extensions
	   2. Retrieve the list of vulkan instance extensions supported on the system
	   3. Stores the extension names in an unordered_set for quick lookup
	   4. Retrieves GLFW-required Vulkan extensions via getRequiredExtensions()
	   5. Checks if each required extensions is supported
	   6. if any required extensions is missing, throws a runtime error */
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






	/* checkDeviceExtensionSupport() checks whether a given Vulkan physical device(GPU) supports all
	   the required extensions enabling additional features i.e swapchain,ray-tracing, multi-gpu-support
	   1. Queries the available device extensions on the given gpu
	   2. Stores the extensions in a vector
	   3. Converts the required extensions into a set for easy lookup
	   4. Removes extensions that are found in the available list
	   5. Return true if all required extensions are available, otherwise false*/
	   //checkDeviceExtensionSupport is also called from isDeviceSuitable as additional check
	bool IkeDeviceEngine::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		//query available device extension with nullptr to retrieve the number of device extensions available on the gpu
		// and stores the available extensions in extensionCount
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		//Retrieve the list of DeviceExtensions by storing the device extension properties
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		//fetch the actual extension names into availableExtensions
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());
		//Converts the deviceExtensions list from vector to set for faster lookup
		// deviceExtensions contains a list of required device extensions i.e VK_KHR_swapchain
		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		//Remove available extension from required list by looping through it and removing matching ones from the set
		// because the set do not take duplicates 
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		//checks if all required extensions were found if all requird extensions is found it returns true else false
		return requiredExtensions.empty();
	}

	/* findQueueFamilies() of type QueueFamilyIndices which is a struct
	is responsible for identifying queue families on a vulkan physical device(VkPhysicalDevice)
	   The function ensures that the device supports both graphics operations and presentation
	 1. Create a QueueFamily Indices object the isComplete method ensures that both graphics
		and presentation queues ara available before selecting the GPU
	 2. Query the Number of Queue Families by initializing queueFamilyCount to 0
		and calls vkGetPhysicalDeviceQueueFamilyProperties with nullptr to get the number of queue families
	 3. Retrieve Queue Family Properties
	  3.a: Allocates a vector of queueFamilies to store all queue family properties then
		   Calls vkGetPhysicalDeviceQueueFamilyProperties again, this time filling queueFamilies
	  3.b: Each VkQueueFamilyProperties contains:
		   queueFlags which specifies the queue capabilities(graphics,compute,transfer,e.t.c)
		   queueCount which is the number of queues available in this family
	 4. Loop through Queue Families
	   4.a: iterate through all queue families(queueFamilies)
	   4.b: int i Keeps track of the index of the queue family
	 5.Check for a Graphics Queue
	   5.a: if the queue has at least one queue(queueCount > 0) and supports graphics
			operations(VK_QUEUE_GRAPHICS_BIT),it is marked as the graphics queue
	   5.b: The index i is stored in indices.graphicsFamily
	   5.c: indices.graphicsFamilyHasValue = true indicates that a grpahics queue was found
	 6. Checks for a Presentation Queue
	  6.a: we initailize a presentSupport of type VKBool32 to false
	  6.b: Calls vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &prsentSupport)
		 b.1: Checks if queue family at index i supports presenting images to the screen
		 b.2: presentSupport is set to VK_TRUE if the queue can present to the surface
	  6.c: if the queue support presentaion(presentSupport is true) and has availble queues(queueCount > 0)
		 c.1: Marks indices.presentFamily = i
		 c.2: Marks indices.presentFamilyHasValue = true
	 7. Stop Searching if a Suitable Device is Found
	  7.a: Calls indices.isComplete(), which returns true only if both:
		   A graphics queue was found and
		   A presentation queue was fount
	  7.b: if both conditions are met, the loop exist early for efficency
	  8. Return the Found Queue Family indices
		 Returns the QueueFamilyIndices struct with the selected queue families
	Note: this function is called and used in isDeviceSuitable()
	  */

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

	/*
	 querySwapChainSupport()
	 Inputs: A vulkan physical device(VkPhysicalDevice device)
	 Output: A swapChainSupportDetails struct containing surface capabilities,supported surface formats and supported presentation modes
	 Steps:
	 0. we declare the SwapchainSupportDetails object by creating it's instance which will store the queried swap chain information
	 1. Queries surface capabilities by calling vkGetPhyscialDeviceSurfaceCapabilitiesKHR, which retrieves capabilities of the surface
		that sores the rseult in details.capabilities this struct contains
		Minimum and maximum number of images in the swap chian
		Minimum and maximum image width and height
		Supported image usage flags(color attachment, transfer destination e.t.c
		Transformation capabilities(rotation,flipping e.t.c)
		Composite alpha modes(handling transperancy)
		The queried capabilities will define the constraints for creating a valid swapchain
	 2. Query supported surface formats by getting the number of available formats
		Retrieves the number of supported surface formats
		The function call with nullptr only fill formatCount without retrieving actual formats
	 2.a Retrieves Available formats using an if condition meaning if the device supports at least one format, it
		 a.1 : Resizes the details.formats vector to hold all available formats
		 a.2 :calls vkGetPhysicalDeviceSurfaceFormatsKHR again, this time storing the acual formats in details in details.formats
	 2.b. each vkSurfaceFormatsKHR contains
		 b.1: format which is the color format(e.g VK_FORMAT_B8G8R8A8_SRGB)
		 b.2: colorSpace which is the colorSpace(e.g VK_COLOR_SPACE_SRGB_NONlINEAR_KHR)
	2.c. if formatCount == 0, the device does not support rendering to the surface
	3 Query supported presentation modes , Gets the number of supported presentation modes if availble, retrieves them by
		 initailizing a uint_32 presentModeCount
		 uses nullptr to only retrieve presentModeCount
	3.a Retrieves Available prsentation modes using an if condition if at least one presentation mode is available:
		a.1: Resizes details.presentModes to hold all availble modes.
		a.2: Calls vkGetPhysicalDeviceSurfacePresentModesKHR again to fill detail.presentModes
	3.b Each VkPresentModesKhR defines how images are presented to the screen, including
		b.1: VK_PRESENT_MODE_FIFO_KHR which is V-sync(default mode)
		b.2: VK_PRESENT_MODE_MAILBOXKHR which is Triple buffering(reduces input lag)
		b.3: VK_PRESENT_MODE_IMMEDIATE_KHR which is No V-Sync(may cause screen tearing)
	3.c if presentModeCount == 0, the device cannot present images to the screen
	4. Returns the filled SwapChainSupportDetails struct
	 This function is a key part of Vulkan's device suitability checks*/
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


	//from here onward every function needs detailed explanation
	// findSupportedFormat function uses Vkformat which is an enum as a type to dictate 
	// how data is stored in memory including the number of components i.e(r,g,b,a)
	// VkImageTiling which is an enum defines how image data is laid out in memory which directly impacts performance and access pattern
	// VkFormatFeaturesFlags also an enum that describes what a particular format can do on an image or buffer 
	// VkFormatproperties which is a struct holding 3 types of VkformateFeatureFlags properties
	// that describes what operations and usage are supported for a specific VkFormat
	
	// we will need a range for loop to traverse the Vkformat then call VkformatProperties and then give it as an 
	// argument to VkGetPhysicalDeviceFormatProperties function which take in the props, the format and physical device as argument
	// the if condition then checks which tiling is available if it is linear or optimal thanks to VkImageTiling
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
   
	// a helper function for selecting a suitable memory type on the GPU based on what kind of memory Vulkan resource(buffer/image) needs
	// when we allocate memories in vulkan we need to specify the memory type index
	// notice the findMemoryType fuction Type is unint32_t which needs to be defined before usage
	// i.e typedef unsigned int unint32_t unlike its counter part uint32 that don't need to be defined
	// findMemorytype has two arguments a uint32_t variable which serves as the type index and VkMemoryPropertyFlags which is an enum
	// and it is used to represent a combination of memory property flags type using bitwise or it is exposed by the vulkan pysical device(GPU)
	// inside the function we encounter VkPysicalDeviceMemoryProperties struct that has two array members and two variable
	// that serves as indexis and describes how much memory the physical device has, how it is structured and what
	// each memory type supports
	// the purpose of this findMemoryType function is when you allocate memory in Vulkan using vkAllocateMemory, you need to 
	// specify a memory index that needs to match two things
	// 1. be allowed by the resource memoryTypeBits, a uint32_t variable member of memoryRequirements struct
	// 2. support the properties you want (i.e VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT_DEVICE_LOCAL_BIT,e.t.c) a member of VkMemoryPropertyFlags
	// Then returns index into VkPhysicalDeviceMemoryProperties.memoryTypes[]
	// The vkGetPhysicalDeviceMemoryProperties gets the memory types and their properties for current GPU
	// The for loop iterate through all memory types supported by the GPU
	// Then the if statement which has two steps a bitshift and equals to condition
	// 1. checks if type i is allowed by the resource by shifting and bitmasking it  by 1 with the help of & operator to test
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
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device_, buffer, bufferMemory, 0);
	}

	VkCommandBuffer IkeDeviceEngine::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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




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
   
	/* a helper function for selecting a suitable memory type on the GPU based on what kind of memory Vulkan resource(buffer / image) needs
	 the purpose of this findMemoryType function is when you allocate memory in Vulkan using vkAllocateMemory, you need to 
	 specify a memory index that needs to match two things
	 1. be allowed by the resource memoryTypeBits, a uint32_t variable member of memoryRequirements struct
	 2. support the properties you want (i.e VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT_DEVICE_LOCAL_BIT,e.t.c) a member of VkMemoryPropertyFlags
	 Then returns index into VkPhysicalDeviceMemoryProperties.memoryTypes[]
	
	 notice the findMemoryType fuction Type is unint32_t which needs to be defined before usage
	 i.e (typedef unsigned int unint32_t) unlike its counter part uint32 that don't need to be defined
	 findMemorytype has two arguments a uint32_t variable typeFilter which serves as the bitmask type that tells you which
	 memory types are acceptable for your allocation (i.e for buffer or image) it comes from Vulkan functions like
	 vkGetBufferMemoryRequirements(...,&memRequirements); and typeFilter serves as uint32_t memoryTypeBits of VkMemoryRequriement struct
	 and VkMemoryPropertyFlags which is an enum, it is a bitmask of VkMemoryPropertyFlagBits
	 tells vulkan the desired propeperties of the memory you are looking for or properties of memory a particular heap has
	 in simple terms it describes how you use a memory block(GPU-only,CPU-accessible,cached, coherent,e.t.c)
	
	 inside the function we encounter VkPysicalDeviceMemoryProperties struct that has two array members and two variable
	 that serves as indexis and describes how much memory the physical device has, how it is structured and what
	 each memory type supports. it is instanciated with memProperties

	 The vkGetPhysicalDeviceMemoryProperties gets the memory types and their properties for current GPU and takes the 
	 physical Device and address of memPropeties as arguments
	 The for loop iterate through all memory types supported by the GPU
	 Then the if statement which has two steps a bit operation and equals to condition
	 1. checks if type i is allowed by the resource by shifting it by 1 which is (1 << i) 
	 then we use the & operator to check weather a specific bit(i) is set in typefilter 
	 notice the left shift (<<) operator shifts the bits position  within a larger integer or binary space
	 && operator which is a normal conditional operator states here that 
	 what ever typefilter is and what memPropeties.memoryType at index i is 
	 notice that memoryTypes which is an array is an instance of memoryType which is a member of the struct VkPhysicalDeviceMemoryProperties
	 memoryType is also a struct that has a member VkMemoryPropertyFlags which is an enum that is instanced to propertyFlags
	 inside memoryType struct. also VkMemoryPropertyFlags is also part of findMemoryType argument
	 with that said in simple laymans term typefilter is an index and properties is holding the bit flags we are trying to access
	 the & operator is used to isolate only the bits that are set in both and == operator means "do all the requested bits exist"
	 if it works it return i and leave the inner-loop and return control flow to the outer-loop to until condition is met
	 if it fails it exit and throws error */

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
/*creatBuffer() function is responsibe for creating Buffer and allocating memory for it
  it is a function with five arguments as its Parameters
  inside the function we call and instance the 
  1. VkBufferCreateInfo that instance two arguments from createBuffer which is size and usage
  we start the VkCreateInfo struct then assign it's members that we need 
  sType a VkStructure Type that is a struct,
  size is VkDeviceSize that is a uint32_t ,
  usage a uint32_t via (typedef VkFlags VkBufferUsageFlags;) and 
  SharingMode which means that we are using only a queue family in this case graphics 
  that is why we using VK_SHARING_MODE_EXCLUSIVE which is assigned 0 in vkSharingMode enum
  2. we then call vkCreateBuffer() function with an if condition and pass in device_ which is a VKDevice handle,
  a reference to the &bufferInfo, nullptr which is a const vkAllocationCallbacks pointer that is a member
  of vkAllocateMemory() function of type VkResult the reason for this is we are not doing any fancy memory
  allocation and Note that instead of using malloc,realloc or free vulkan uses vkAllocateMemory function for that
  then we compare it to Vk_SUCCESS if it returns not succesful it throws a runtime error if not we proceed to 
  3. The VkMemoryRequirements that we instance with memRequirements which serves for vulkan to determine how much
  memory we need to allocate for a resource (i.e buffer or image) and what kind of memory we can use
  4. we then use vkGetMemoryRequiremtents() to query the memory requirement for a given vulkan buffer object
  this function tells us how much memory we need to allocate and the constraints for that memory to be compatible
  with the buffers. its arguments are the device_ which is the vulkan logical device that owns the buffer
  the buffer an instance of vkBuffer a vulkan object, whose memory requirements we want,and a reference to 
  the VkMemoryRequirements via memRequirements because inside the vkGetBufferMemoryRequirements() function 
  vkMemoryRequirements originally a struct is instanced as a pointer member of VkMemoryRequirements()
  the reason why we are referencing it
  5. we then instance a VkMemoryAllocateInfo struct with allocInfo that is used to specify parameters for
  allocating device memory in vulkan. the idea is that we fill out the struct and pass it to vkAllocateMemory()function
  the VkMemoryAllocateInfo members are the sType which is needed to identify vulkan structures
  the Allocation size which we assign the VkMemoryRequirement member size to, via memRequirement.size
  the memoryTypeIndex a uint32_t type which is used to specify which type of memory to allocate,
  chosen based on required memory properties you will notice that we assigned findMemoryType() to it
  you will notice that its argument this time is VkMemoryRequirements.memoryTypebits member which is a uint32_t type
  and VkMemoryPropertyFlags struct which we instanced as properties
  6.Then we use an if statement to call vkAllocateMemory() and its parameters are the vulkan device_,
   a reference to the AllocateInfo struct, a null pointer that serves as the VkAllocationCallbacks pointer
   and a reference to the VkDeviceMemory handle via bufferMemory we then check if it is not successfull
   throw an error else we bind it to vkBindBufferMemory() which takes the VkDevice handle device_,the VkBuffer and
   zero which is an offset into the memory block it is commonly 0 unless we are using sub-allocation
   the function serves as a way to bind a block of device memory which is previously allocacted to 
   a Vulkan buffer object

*/

	void IkeDeviceEngine::createBuffer(
		VkDeviceSize size,                   //the size in bytes of the buffer to create
		VkBufferUsageFlags usage,            // how the buffer will be used e.g vertx buffer, uniform buffer
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
		vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;           // the size of the memory blocks you want to allocate on the device
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);  // index ino the physical devices memory types array

		if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device_, buffer, bufferMemory, 0);  // links the memory block to the buffer
	}

	/*The beginSingleCommand() a function with no arguments is of type VkCommandBuffer which is a vulkan handle 
	  is a helper or utility function that allocates and begins a primary command buffer intended for a single-term use 
	  like a one-time copy operation or transition it is typically used for short lived operations, 
	  such as copying to buffer data or transition image layouts during initialization
	  1. we instanced the VkCommandBufferAllocateInfo struct via alloInfo which job is tell vulkan how many command buffers
	     to allocate and from which command pool
		 .sType is an instance of VKStructure Type that sets the type of struct required by vulkan
		 .level  is assigned VK_COMMAND_BUFFER_LEVEL_PRIMARY meaning that we are telling vulkan to submit directly to a queue
		 unlike VK_COMMAND_BUFFER_LEVEL_SECONDARY which is used differently so here we allocating a primary command buffer
		 .commandPool is assigned commanPool instance of VKCommandPool a vulkan handle  which Specifies the command pool 
		   from which the buffer should be allocated
		 .commandBufferCount is assigned 1 meaning only 1 buffer is being allocated

	  2. VkCommandBuffer is the handle to the command buffer you want to begin recording which we have already allocated 
		 via vkAllocateCommandBuffers() function
	  3. vkAllocateCommandBuffers() is a vulkan function that allocates one or more command buffer from a command pool
	     and fills the commandBuffer with the allocated command buffer handle it takes the device_, 
         a reference to the allocInfo struct and a pointer to the commandBuffer handle
	  4. we then instanced VkCommandBufferBeginInfo struct via beginInfo which is used to control how a command
	     buffer is recorded and how it interacts with other command buffers and then we specify it's parameters
		 for beginning a command buffer recording with sType which is an instance of VKStructure Type and flags which is a bitmask
	  5. we call vkBeginCommandBuffer() which is a vulkan function that begins recording commands into the command buffer
		 it takes the commandBuffer handle and a reference to the beginInfo struct
	  6. Finally, the function returns the commandBuffer handle which is now ready for recording commands
	  */
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

	/* endSingleTimeCommands() is a function with one argument of type VkCommandBuffer which is a vulkan handle
	   it is used to end the recording of commands in a single-use command buffer and submit it to the graphics queue
	   this is used together with beginSinlgeTimeCommands() notice that VkCommandBuffer is it's argument unlike the 
	   beginSingleTimeCommand which called VkCommandBuffer inside it's function
	   1. we call vkEndCommandBuffer() which is a vulkan function that ends the recording of commands in the command buffer
	      it ends recording commands into the buffer,it must be called before we can submit to a queue and if the buffer
		  isn't properly ended, submission will fail
	   2. we then instanced VkSubmitInfo struct via submitInfo which is used to specify parameters for submitting command buffers to a queue
		  .sType is an instance of VKStructure Type that sets the type of struct required by vulkan
		  .commandBufferCount is set to 1 meaning we are submitting only one command buffer
		  .pCommandBuffers is a pointer to an array of command buffers to be submitted, in this case, 
		  it points to the address of commandBuffer the reason we are referencing it
	   3. we call vkQueueSubmit() which is used to send recorded command buffer to the graphics queue so that the GPU can execute them
		  it takes the graphicsQueue_ an instance of VkQueue handle where work is submitted,
		  the number of submition we are making in this case is 1, a reference to submitInfo struct and 
		  a fence handle which is set to VK_NULL_HANDLE
	   4. we then call vkQueueWaitIdle() with graphicsQueue_ as an argument. it waits for the graphics queue to become idle, 
	      ensuring all submitted commands are completed before proceeding meaning it blocks the CPU until all previously
		  submitted GPU work on the given VkQueue is fully completed
	   5. Finally, we free the command buffer using vkFreeCommandBuffers() which releases the resources associated with the command buffer
	      and returns their memory to the pool from which they were allocated
		  it takes the device_, commandPool and number of command buffers(1) and a pointer to the commandBuffer handle
	*/
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
	/* copyBuffer() is a utility/helper function that performs a GPU-side memory copy from one vulkan buffer
	   (srcBuffer) to another (dstBuffer) using a temporary (single-use) command buffer
	   1. we start by objectifying VkCommandBuffer struct with commandBuffer and assign beginSingleTimeCommand() to it
	   the reason for this is the beginSingleTimeCommands() function allocates and begin recording a primary command
	   buffer intended for one-time submission
	   2. we then instance the VkBufferCopy a struct  with it's parameters which are all an instance VkDeviceSize
	      that is (typedef uint64_t VkDeviceSize) 
	      .srcOffset is the byter offset from the start of the source buffer which defines a region of memory to read from 
	      .dstOffset which is the start of the destination buffer where the data will be written 0th index ,
		  .size is the total number of bytes to copy from srcOffset to dstOffset this defines the length of the
		  memory region being transfered

	   3. then we call vkCmdCopyBuffer() with its parameters it copy buffersize bytes from staging buffer to the deviceBuffer
	      the commandBuffer is an instance of VkCommandBuffer which we are recording the copy command into and it must be
		  in a recording state
		  the srcBuffer which is the GPUbuffer we are copying from and must be created with VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		  ths dstBuffer which is the GPUbuffer we are copying into and must be created with VK_BUFFER_USAGE_TRANSFER_DST_BIT
		  1 is the number of regions to copy meaning number of VkBufferCopy structure
		  and reference to the copyRegion
		  then we call endSingleTimeCommands() with commandBuffer as argument this 
	   */
	void IkeDeviceEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; //Optional
		copyRegion.dstOffset = 0; //Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}
	/*the copyBufferToImage is a function that help us copy data from a Vulkan Buffer to a Vulkan Image, which is necessary and 
	  commonly used for transfering textures data into GPU memory and take in five parameters
	   buffer which contains the source image data i.e texture pixtels
	   image which will recieve the data
	   width and height dimensions of the image
	   layerCount Number of image array layers(useful for 3d textures or cube maps)
	   now we are going to use the helper functions to begin and end recording
	   we star with beginSingleTimeCommands() that is assigned to commandBuffer which is an instance of VkCommandBuffer
	   that allocates and begins recording with VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	   the VkBufferImageCopy struct is instanced by region which will describe the region of data that should be copied
	   .bufferOffset = 0 starts at the beginning of the buffer
	   .bufferRowLength = 0 tell vulkan that rows in the buffer are tightly packed i.e use imageExtent.width as row length
	   .bufferImageHeight = 0 also tightly vertically packed 
	   .imageSubresource which is a member of VkBufferImageCopy is also a struct and it defines which layers and mipmap level
	    of image to target
		.aspectMask is for color images
		.miplevel represents a specific level or detail of a texture image it is used to optimize rendering performance and
		 improve visual quality assignig 0 to it means we will need to generate other miplevel manually or via gpu later
		 because 0 is just copying to the base
		.baseArrayLayer = 0 means we are starting from the first array layer
		.layerCount is the total number of layers to update
		.imageOffset tells us to start writing at the top-left corner of the image where (x=0,y=0,z=0)
	    .imageExtent Defines the full size of the region to write to and the Depth is 1 which means it is a 2d image
		then we call vkCmdCopyBufferToImage() which is use to copy data from a buffer to an image typically used for
		uploading texture data from cpu to gpu it's parameters are
		commandBuffer the command buffer we're recording into
		buffer the source buffer
		image the destination buffer
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL the must be in this layout before copying
		1 the number of regions to copy and 
		a reference to region which is a pointer to the struct we set up
		then we call endSingleTimeCommands with the commandBuffer as an argument which
		ends the command buffer,submits it to a graphic or transfer queue, waits for completion and frees the command buffer


	   */
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
	/*createImageWithInfo() is a helper function that creates vulkan image,allocates memory for it and 
	  binds the memory to image it has four parameters
	  VkImageCreateInfo struct that was made a const ,reference and instanced describes and create
	  image resource via VkImage in vulkan which is a memory resource that can represent texture,render targets
	  depth buffers or storage images.
	  VkMemoryPropertyFlags represent flags for memory properties (i.e VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	  VkImage output vulkan image handle
	  VkDeviceMemory output vulkan device memory handle
	  then we call vkCreateImage() that will create a vulkan VkImage using imageInfo,it stores the resulting
	  image handle with image ,because it needs a logical device handle so we also include device_ and if it fails and exception is thrown
	  then we get the memory requirments via VkMemoryRequirements
	  then we query vkGetImageMemoryRequiremtens() which is to query how much GPU memory is required for Vulkan image(created with vkCreateImage)
	  we then Allocate memory via VkMemoryAllocateInfo struct
	  and check if vkAllocateMemory() memory which is used for allocating raw GPU memory from the logical device
	  it first request the Vulkan driver via device_ to reserve a contiguous memory block from the devices memory pool
	  a reference to VkMemoryAllocateInfo struct which is tells vulkan how much memory and what type we want
	  null for the pnext pointer which is reserved for extensions
	  and VkDeviceMemory reference which vulkan will fill with the handle to the newly allocated memory block and
	  which will be passed later to vkBindImageMemory()
	  we use an if statement to call vkBindImageMemory() which is used to bind the allocated memory to the image
	  we call the device_,bind the imageMemory to image and 0 is the offset means that the memory starts from the begining

	  */
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




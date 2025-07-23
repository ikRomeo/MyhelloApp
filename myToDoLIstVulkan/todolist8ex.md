## ikDevice.hpp file
#ifndef IKEDEVICEENGINE_HPP
#define  IKEDEVICEENGINE_HPP

#include "ikWindow.hpp"

#include <vector>
#include <string>


namespace ikE{

  class IkeDeviceEngine{
    public:
      
        //QueueFamilies
        struct QueueFamilyIndices {
            uint32_t graphicsFamily;
            uint32_t presentFamily;

        };


        // macro enabling validation layer in debug mode
       #ifdef NDEBUG
            const bool enableValidationlayers = false;
       #else
            const bool enableValidationlayers = true;
      #endif
            

      IkeDeviceEngine(IkeWindow& window);
      ~IkeDeviceEngine();
//not cobyable 
      IkeDeviceEngine(const IkeDeviceEngine&) = delete;
      void operator=(const IkeDeviceEngine&) = delete;
//not movable
      IkeDeviceEngine(const IkeDeviceEngine&&) = delete;
      IkeDeviceEngine& operator=(IkeDeviceEngine&&) = delete;

      //getter functions
      VkDevice device() { return device_; };
      VkQueue graphicsQueue() { return graphicsQueue_;};
      VkQueue presentQueue() { return presentQueue_;};


      //used for commandpool
      QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); };

      VkPhysicalDeviceProperties properties;



  private:
      void createInstance();
      void setupDebugMessenger();

      void pickPhysicalDevice();
      void createLogicalDevice();


    //helper functions
      bool isDeviceSuitable(VkPhysicalDevice device);
      std::vector<const char*> getRequiredExtensions();
      bool checkValidationLayerSupport();
      QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
      void hasGlfwRequiredInstanceExtensions();
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);




      //handles and objects
      VkInstance instance;
      VkDebugUtilsMessengerEXT debugMessenger;
      IkeWindow& window;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

      //
      VkDevice device_;
      VkQueue graphicsQueue_;
      VkQueue presentQueue_;


      const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
      //NOTE
      const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    
  };


}//namespace 
#endif 


#ikDevice.cppfile


#include "ikDeviceEngine.hpp"

//std headers
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
	//check if the device is suitable for the operations we need them 
	bool IkeDeviceEngine::isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);
		return true;
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



	void IkeDeviceEngine::setupDebugMessenger() {
		if (!enableValidationlayers) return;
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


	//this function need explanation
	void IkeDeviceEngine::hasGlfwRequiredInstanceExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


		std::cout << " yeah available extensions:\n";
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
			available.insert(extension.extensionName);
		}

				std::cout << "Required extensions:\n";
				auto requiredExtensions = getRequiredExtensions();
				for (const auto &required : requiredExtensions) {
					std::cout << "\t" << required << std::endl;
					if (available.find(required) == available.end()){
				      throw std::runtime_error("Missing required glfw extension");
		              }
				   }

	}

	ikE::IkeDeviceEngine::QueueFamilyIndices IkeDeviceEngine::findQueueFamilies(VkPhysicalDevice device) {
	
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

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;


	}


   
} //namespace




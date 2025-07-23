
ikDeviceEngine.hpp file

#ifndef IKEDEVICEENGINE_HPP
#define  IKEDEVICEENGINE_HPP

#include "ikWindow.hpp"

#include <vector>
#include <string>


namespace ikE{
  class IkeDeviceEngine{
    public:

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

  private:
      void createInstance();
      void setupDebugMessenger();

      void pickPhysicalDevice();


    //helper functions
      std::vector<const char*> getRequiredExtensions();
      bool checkValidationLayerSupport();
      void hasGlfwRequiredInstanceExtensions();
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);





      VkInstance instance;
      IkeWindow& window;


      const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
    
  };


}//namespace 
#endif 



ikDiviceEngine.cpp file

#include "ikDeviceEngine.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
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
		
	}
    //destructor
	IkeDeviceEngine::~IkeDeviceEngine() {
        
        if(enableValidationlayers){
           DestroyDebugUtilsMessengerEXT(instance, debugMessenger,nullptr)
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
		if (enableValidationlayers){
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		}
		return extensions;
	}



	void IkeDeviceEngine::hasGlfwRequiredInstanceExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		
		std::vector<VkExtensionProperties> extensions(extensionCount);
		
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


		std::cout << " yeah available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
			//available.insert(extension.extensionName);
		}


			
		}
   } //namespace




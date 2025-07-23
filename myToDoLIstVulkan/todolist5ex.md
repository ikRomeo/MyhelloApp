##ikDeviceEngine.hpp file
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


    //helper functions
      std::vector<const char*> getRequiredExtensions();
      bool checkValidationLayerSupport();
      void hasGlfwRequiredInstanceExtensions();





      VkInstance instance;
      IkeWindow& window;


      const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
    
  };


}//namespace 
#endif 





##ikDeviceEngine.cpp file
#include "ikDeviceEngine.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>


namespace ikE {



	IkeDeviceEngine::IkeDeviceEngine(IkeWindow& window) : window(window) {
		createInstance();
	}
	IkeDeviceEngine::~IkeDeviceEngine() {
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


		std::cout << "available extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
			//available.insert(extension.extensionName);
		}

		//std::cout << "Required extensions:\n";
		//auto requiredExtensions = getRequiredExtensions();
		//for (const auto& required : requiredExtensions()) {
			
		}
   } //namespace






ikDeviceEngine.hpp file
#ifndef IKEDEVICEENGINE_HPP
#define  IKEDEVICEENGINE_HPP

#include "ikWindow.hpp"
namespace ikE{
  class IkeDeviceEngine{
    public:



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
      VkInstance instance;
      IkeWindow& window;
    
  };


ikDeviceEngine.cpp file

#include "ikDeviceEngine.hpp"
#include <stdexcept>
#include <iostream>


namespace ikE {



	IkeDeviceEngine::IkeDeviceEngine(IkeWindow& window) : window(window) {
		createInstance();
	}
	IkeDeviceEngine::~IkeDeviceEngine() {
		vkDestroyInstance(instance, nullptr);
	
		glfwTerminate();
	}


	void IkeDeviceEngine::createInstance() {

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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;


		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance");

		}
	}
}
 
First_App.hpp file

#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikWindow.hpp"
#include "ikPipeline.hpp"
#include "ikDeviceEngine.hpp"
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run();

	private:
		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
		IkeDeviceEngine ikeDeviceEngine{ ikeWindow };
        ikePipeline ikePipeline{"frag.spv","vert.spv"};
	};

} //namepace
#endif //header guard
 


After Compilation we implement checking for extension support

ikDeviceEngine.hpp file

#ifndef IKEDEVICEENGINE_HPP
#define  IKEDEVICEENGINE_HPP

#include "ikWindow.hpp"
namespace ikE{
  class IkeDeviceEngine{
    public:



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


      void hasGlfwRequiredInstanceExtensions();





      VkInstance instance;
      IkeWindow& window;
    
  };


}//namespace 
#endif 


ikDevicEngine.cpp file

#include "ikDeviceEngine.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>


namespace ikE {



	IkeDeviceEngine::IkeDeviceEngine(IkeWindow& window) : window(window) {
		createInstance();
	}
	IkeDeviceEngine::~IkeDeviceEngine() {
		vkDestroyInstance(instance, nullptr);
	
		glfwTerminate();
	}


	void IkeDeviceEngine::createInstance() {

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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;


		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance");

		}
		hasGlfwRequiredInstanceExtensions();
	}





	void IkeDeviceEngine::hasGlfwRequiredInstanceExtensions() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		// we will have to include <vector header
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());


		std::cout << "available extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}
   }

}//namespace




==the output after compilation==

available extensions:
        VK_KHR_device_group_creation
        VK_KHR_external_fence_capabilities
        VK_KHR_external_memory_capabilities
        VK_KHR_external_semaphore_capabilities
        VK_KHR_get_physical_device_properties2
        VK_KHR_get_surface_capabilities2
        VK_KHR_surface
        VK_KHR_surface_protected_capabilities
        VK_KHR_win32_surface
        VK_EXT_debug_report
        VK_EXT_debug_utils
        VK_EXT_surface_maintenance1
        VK_EXT_swapchain_colorspace
        VK_NV_external_memory_capabilities
        VK_KHR_portability_enumeration
        VK_LUNARG_direct_driver_loading
Vertex Shader Code size:424
Fragment Shader Code size:1348

C:\Users\ikrom\source\repos\MYhelloApp\x64\Debug\MYhelloApp.exe (process 17420) exited with code 0 (0x0).
Press any key to close this window . . .





==Validation layer==

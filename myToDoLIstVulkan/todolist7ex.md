ikDeviceEngine.hpp file


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


      

      VkPhysicalDeviceProperties properties;



  private:
      void createInstance();
      void setupDebugMessenger();

      void pickPhysicalDevice();


    //helper functions
      bool isDeviceSuitable(VkPhysicalDevice device);
      std::vector<const char*> getRequiredExtensions();
      bool checkValidationLayerSupport();
      QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
      void hasGlfwRequiredInstanceExtensions();
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);




      //handles and objects
      VkInstance instance;
      IkeWindow& window;
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

      const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
    
  };


}//namespace 
#endif 


the ikDeviceEngine.cpp file



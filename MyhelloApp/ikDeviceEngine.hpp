#pragma once
#ifndef IKDEVICEENGINE_HPP
#define  IKDEVICEENGINE_HPP

#include "ikWindow.hpp"

#include <vector>
#include <string>

//#include <vulkan/vulkan.h>



//notice that there are two struct that is outside the class and like the class they are all undernamespace ikE
namespace ikE{
    /*SwapChainSupportDetails stores capabilities, formats, and presentation modes for a Vulkan surface.
      It helps determine whether a GPU can properly render and present images.
      Used in isDeviceSuitable to check if the swap chain is adequate for rendering.*/ 
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    /*This struct stores the queue family indices
      graphicsFamilyHasValue → Tracks if the GPU has a graphics queue.
      presentFamilyHasValue → Tracks if the GPU has a presentation queue.
      isComplete() → Returns true only if both are found.
      Used in device suitability checks for Vulkan initialization.
      This check ensures that the GPU can both render graphics and present them to the screen, which is essential for Vulkan applications.*/
    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;

        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };



  class IkeDeviceEngine{
    public:

        // macro enabling validation layer in debug mode
       #ifdef NDEBUG
            const bool enableValidationlayers = false;
       #else
            const bool enableValidationlayers = true;
      #endif
            

      IkeDeviceEngine(IkeWindow& window);
      ~IkeDeviceEngine();
//not copyable 
      IkeDeviceEngine(const IkeDeviceEngine&) = delete;
      IkeDeviceEngine& operator=(const IkeDeviceEngine&) = delete;
//not movable
      IkeDeviceEngine(const IkeDeviceEngine&&) = delete;
      IkeDeviceEngine& operator=(IkeDeviceEngine&&) = delete;

      //getter functions
      VkCommandPool getCommandPool() { return commandPool; };
      VkDevice device() { return device_; };

      //surface() needs explanation
      VkSurfaceKHR surface() { return surface_;}
      VkQueue graphicsQueue() { return graphicsQueue_;};
      VkQueue presentQueue() { return presentQueue_;};

      SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); };
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);



      //used for commandpool
      QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); };
      //findSupportedFormat needs explanation
      VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

      //Buffer Helper functions
      //public createcommandpool
      void createCommandPool();

      void createBuffer(
          VkDeviceSize size,
          VkBufferUsageFlags usage,
          VkMemoryPropertyFlags properties,
          VkBuffer& buffer,
          VkDeviceMemory& bufferMemory);
      VkCommandBuffer beginSingleTimeCommands();
      void endSingleTimeCommands(VkCommandBuffer commandBuffer);
      void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
      void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

      void createImageWithInfo(
          const VkImageCreateInfo& imageInfo,
          VkMemoryPropertyFlags properties,
          VkImage &image,
          VkDeviceMemory &imageMemory);

      VkPhysicalDeviceProperties properties;



  private:
      void createInstance();
      void setupDebugMessenger();
      //
      void createSurface();
      void pickPhysicalDevice();
      void createLogicalDevice();
      //
      //void createCommandPool();


    //helper functions
      bool isDeviceSuitable(VkPhysicalDevice device);
      std::vector<const char*> getRequiredExtensions();
      bool checkValidationLayerSupport();
      QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
      void hasGlfwRequiredInstanceExtensions();
      bool checkDeviceExtensionSupport(VkPhysicalDevice device);
      SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
     




      //handles and objects
      VkInstance instance;
      VkDebugUtilsMessengerEXT debugMessenger;
      IkeWindow& window;
      /*VkPhysicalDevice Represents the actual GPU hardware e.g your NVIDIA RTX,AMD Radeon, or Intel GPU
        we set it to vk_null_handle before it is properly created or assigned*/
      VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
      VkCommandPool commandPool;

      //getters headers
      /*VkDevice is the logical device a software connection to the GPU, it
      defines how we communicate with the hardware(queues,features, extensions,e.t.c*/
      VkDevice device_;
      //
      VkSurfaceKHR surface_;
      VkQueue graphicsQueue_;
      VkQueue presentQueue_;





      const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
      /*Vk_KHR_SWAPCHAIN_EXTENSION_NAME is essential for rendering images to a display surface
        it enables image presentation in vulkan by interfacing with the window system and 
        swapping of framebuffers
      */
      const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    
  };


}//namespace 
#endif     

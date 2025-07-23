# swap chain
because vulkan by default does not have a frame buffer we will need A swapchain which are a queue of images that are waiting to be presented to the screen. the idea is that our application will acquire such an image to draw to it and then return it to the queue
A swapchain is a series of images(framebuffers) used for rendering and displaying frames efficiently.
it manages the synchronization between
 * The Gpu rendering process
 * The display system(e.g a monitor)
Instead of rendering directly to the screen, Vulkan renders frames into swapchain images, which are later presented to the screen in an orderly fashion
#Why is Swapchain Needed
 * Double/triple buffering: it prevents tearing and allows smoother animation
 * Synchronization: Cordinates gpu execution and presentation
 * frame rate Management: handles refresh rates and V synchronization

The STEPS:
 1. check for swap chain support
 2. Enable device extensions
 3. Query details of swap chain support
 4. Choose the right settings for the swap chain
 5. Create the swapchain
 6. Retrieve the swap chain images

##Step 1:
#Vk_KHR_SWAPCHAIN_EXTENSION
 vulkan does not include swapchain functionality in it's core Specification because it aims to be a cross-platform API, and different platforms have different windowing systems

 so we will need to add it in the ikDeviceEngine.hpp file to ensure extension is used
##ikDeviceEngine.hpp file
# Private section:

                       const std::vector<const char*> deviceExtenstions = {Vk_KHR_SWAPCHAIN_EXTENSION };
the Vk_KHR_SWAPCHAIN_EXTENSION provides
1.Swapchain creation and management(vkCreateSwapchainKHR) and (vkDestroySwapchainKHR)
2.Image acquisition(vkAcquireNextImageKHR)
3.Queue-based presentation(vkQueuePresentKHR)

To use swapchain extensions we must Check for support which is the VK_KHR_SWAPCHAIN meaning we need
to define a function called 


inside the Private section of ikDeviceEngine.hpp file under the helper function we will declare
# Private section
      
       bool isDeviceSuitable(VkPhysicalDevice device);
then we will define it in the ikDeviceEngine.cpp file
## ikDeviceEngine.cpp file
The function isDeviceSuitable verifies whether a VulkanGpu(physicalDevice) meets the necessary requirements for use specifically, it ensures
1. The device has queue families for rendering and presenting.via findQueuFamilies()
2. The required Vulkan extensions are available.via checkDeviceExtensionSupport()
3. The swap chain is adequate for rendering.
4. The device supports anisotropic filtering.
If all these conditions are satisfied, the function returns true, meaning the device is suitable for Vulkan rendering. Otherwise, it returns false


            bool IkeDeviceEngine::isDeviceSuitable(VkPhysicalDevice device) {
	              QueueFamilyIndices indices = findQueueFamilies(device);
	
	              bool extensionsSupported = checkDeviceExtensionSupport(device);
we intialized a bool swapChainAdequate to false meaning if at least one valid format and presentation mode exist, it becomes true else it becomes false,meaning the device cannot present images properly

	              bool swapChainAdequate = false;
	              if (extensionsSupported) {
		                  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		                  swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

	                }
	              VkPhysicalDeviceFeatures supportedFeatures;
	              vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	             return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
                      }


 -[] then inside the struct QueueFamilyIndices we will need two members
     of type bool and a function of type bool
     bool graphicsFamilyHasValue that tracks if the Gpu has graphics queue 
     bool presentFamilyHasValue that tracks if the Gpu has a presentation queue
     and isComplete also of type bool that Returns true only if both are found
     set to false.
     
### Code Example
               
               struct QueueFamilyIndices{ 
                      uint32_t graphicsFamily;
                      uint32_t presentFamily;

                      bool graphicsFamilyHasValue = false;
                      bool presentFamilyHasValue = false;
                      bool isComplete(){return graphicsFamilyHasValue && presentFamilyHasValue;}
                      };
                      

                 bool checkDeviceExtensionSupport(VkPhysical device);
then define it in the ikDeviceEngine.cpp file
##ikDeviceEngine.cpp file
checkDeviceExtensionSupport() checks whether a given Vulkan physical device(GPU) supports all
   the required extensions enabling additional features i.e swapchain,ray-tracing, multi-gpu-support
   1. Queries the available device extensions on the given gpu
   2. Stores the extensions in a vkEnumerateDeviceExtensionProperties
   3. Converts the required extensions into a set for easy lookup
   4. Removes extensions that are found in the available list
   5. Return true if all required extensions are available, otherwise false
   checkDeviceExtensionSupport is also called from isDeviceSuitable as additional check
   
                bool IkeDeviceEngine::checkDeviceExtensionSupport(VkPhysicalDevice device) {
query available device extension with nullptr to retrieve the number of device extensions available on the gpu    
    and stores the available extensions in extensionCount
    
             uint32_t extensionCount;
		     vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
Retrieve the list of DeviceExtensions by storing the device extension properties
	         
             std::vector<VkExtensionProperties> availableExtensions(extensionCount);
fetch the actual extension names into availableExtensions
	         
             vkEnumerateDeviceExtensionProperties( device, nullptr,&extensionCount,availableExtensions.data());
Converts the deviceExtensions list from vector to set for faster lookup  deviceExtensions contains a list of required device extensions i.e VK_KHR_swapchainstd
             
             std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
Remove available extension from required list by looping through it and removing matching ones from the set because the set do not take duplicates 
     
             for (const auto& extension : availableExtensions) {
                    requiredExtensions.erase(extension.extensionName); }
checks if all required extensions were found if all requird extensions is found it returns true else false

           return requiredExtensions.empty(); }
           
##Step2:           
we create a struct outside the class object
##ikDeviceEngine.hpp
SwapChainSupportDetails stores 
capabilities, which is VKSurfaceCapabilitiesKHR 
  * it Stores general properties of the swap chain surface that includes
     * Minimum and Maximum number of images in the swap chain
     * Minimum and Maximum images width/height
     * image usage flags(e.g color attachment, transfer destination)
     * Transfor capabilities(e.g rotation, flipping)
     * Composite alpha modes(handling transparency)
     It is retrieved using (vkGetPhysicalSurfaceCapabilitiesKHR(physicalDevice,surface, &capabilites)
  
 formats which is held in a std::vector container VkSurfaceFormatKHR
     * A list of supported surface formats for rendering  Each VKSurfaceFormantKHR contains
     * the color format (e.g VK_FORMAT_B8G8R8A8_SRGB) 
     * the color space (e.g, VK_COLOR_SPACE_SRGB_NONlINEAR_KHR)
 and presentation modes which is also held inside a std::vector container
     * A list of supported presentation modes
     * Determines how images are displayed to the screen (e.g V-sync, immediate redeing, mailbox mode)
        common values are
             VK_PRESENT_MODE_FIFO_KHR = V-Sync (default)
             VK_PRESENT_MODE_MAILBOX_KHR = Triple buffering
             VK_PRESENT_MODE_IMMEDIAT_KHR = No V-sync(tearing possible)

#summary
swapchainSupportDetails store capabilities,formats and presentation modes for a vulkan surface
 it helps determine whether a gpu can properly render and present images
 it is Used in isDeviceSuitable() to check if the swap chain is adequate for rendering
 it is a critical part of vulkan's device selection process
 
              struct SwapChainSupportDetails{
                      VkSurfaceCapabilitiesKHR capabilities;
                      std::vector<VkSurfaceFomatKHR> formats;
                      std::vector<VkPresentModeKHR> presentsModes;
              };

then we create a function with type SwapChainSupportDetails which we will use to query the swapchain                   
# under the helper function of ikDeviceEngine.hpp file
 we will declare that will help us query the swapchain details
                   
                   SwapChainSupportDetails querySwapChainSupport(VkPhyscialDevice device);
inside ikDeviceEngine.cpp file
## ikDeviceEngine.cpp
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
 This function is a key part of Vulkan's device suitability checks


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
we have to extend the isDeviceSuitable() to use this function to verify that the SwapChainSupport is Adequate by adding this to the function


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

# Choosing the right settings for the swap chain
if the swapChainAdequate conditions were met then the support is definately sufficient
but there may still be many different modes of varying optimality
we will now write a couple of funcitons to find the right settings for best possible
swapchain
There are three type of settings to determine:
1. Surface formats(color depth)
2. Presentation mode(conditions for "Swapping" images to the screen)
3. Swap extent(resolution of images in swapchaint)
for each of these settings we'll have an ideal value in mind that we'll go with
if it is available and otherwise we'll create some logic to find the next best thing

# surface format: Each VkSurfaceFomatKHR entry contains a format and a colorspace members
#                 The format member specifies the color channels and types e.g 
#                 VK_FORMAT_B8G8R8A8_SRGB means that we stor the B,G,R and Alpa channels
#                 in that order with an 8bit unsigned integer for a total of 32bits per pixel

# The colorSpace : member indicates if the SRGB color space is supported or not using
#                  the VK_COLOR_SPACE_SRGB_NONlINEAR_KHR flag which is pretty much the
#                  standard color space for images,like textures





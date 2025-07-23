# ==Message callback==
## message callback is a mechanism used to recieve and handle debug messages and validation layers messages
## generated through vulkan API

 ==why we need it==
## we need a way to print out debug messages to the Standard Output which we enabled through setting the validation layer
## by default the validation layers will print debug messages to the Standard Output by default 
## but we can also handle them by providing an explicit callback() function in the program

==how to set up a callback() in the program to handle messages and associated details==
## - [] we have to set up a debug messenger with a callback using the *VK_EXT_debug_utils extension*
###    1. we will create a *getRequiredExtension()* function that will return the requird list of
###       extensions based on whether validation layers are enabled or not
# ProtoType 

           GLFWAPI const char** glfwGetRequiredInstanceExtenstion(unit32_t* count);
          
##         This function returns the address of a specified Vulkan instance function
##         it returns the vulkan instance required by GLFW for creating surfaces for glfw windows
##         if succesfull on return the list will contain *VK_KHR_surface*
##         the parameter list: which is uint32_t* count is where to store the number of extensions
##                             in the returned array. it is set to zero if an error occured
##         Returns: it returns an Array of ascii encoded Extension names or Null if an error is encountered
##         GLFWAPi: is a macro that is  used to declare public API functions for export from DLL, shared library, dynamic library
==ikDeviceEngine.hpp file==
#Private Section== of IkeDeviceEngine class where the helper functions are located
### - [] we will declare *getRequiredExtenstions()* function typecast it to a const char pointer 
###       and put it inside a dynamic array container via std::vector
# Code Example:
            
               std::vector<const char*> getRequiredExtenions();

==ikDeviceEngine.hpp file==
underneath the checkValidationLayerSupport() 
### -[] we will define the getRequiredExtensions() function which has no argument
# Code Example:
	          
              std::vector<const char*> IkeDeviceEngine::getRequiredExtensions() { 
              uint32_t glfwExtensionCount = 0;
              const char** glfwExtensions;
              glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
              std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);


             if (enableValidationlayers){
                     extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                    }
            return extensions;
            }
            
            
 This Code snippet: 
                  glfwGetRequiredInstanceExtensions retrieves an array of extensions and their count, and the std::vector extensions stores them for further processing.
                  then the if-Condition retrieves and prepares the list of required extensions, including optional validation layers if enabled. Here's a detailed explanation:
## Notice: that inside the getRequiredExtensions() 
##         we used the first three lines of code before, when we needed GLFW built-in function
##         that returns the extension inside *CreateInstance()* function
##         because the first three lines of code are used inside the getRequireExtensions()
##         we will need to remove the three lines of code from the *CreateInstance()* after we a done here
######     with the fourth line of code we have a Complete Two way Process of querring data
                 
###       - [] we initialized a variable of type uint32_t *glfwExtensionCount* to 0 
###       - [] we declare a pointer to a pointer variable *gflwExtensions* of type char and a const as a modifier meaning it is immutable
###       - [] we assign *glfwGetRequiredInstanceExtensions()* with reference to *glfwExtensionCount* as it's argument to *glfwExtensions*
###       - [] because we want to take the first and last element of the array we will need to use this code syntax 
                  std::vector<const char*>(glfwExtensions,glfwExtensions + glfwExtensionCount);
#                Summary:
###              The line creates a std::vector that contains all the elements of the array starting at glfwExtensions and ending before (glfwExtensions + glfwExtensionCount). 
###              This is commonly used when working with APIs like Vulkan, where extensions are provided as an array of strings. 
###              *(glfwExtensions + glfwExtensionCount)*: is A pointer to one past the last element of the array. This is used to specify the range of elements to include in the vector.
###              std::vector in this case is used as a range where the first to last elments are the bounds or top and down ceiling
###       - []  now in the if-Condition
###             Context:
###             enableValidationLayers: is A boolean variable that determines whether to include debug utilities for Vulkan validation layers.
###                                     Validation layers are used during development to identify issues with Vulkan API usage.
###             *extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME):* Adds the debug utility extension (VK_EXT_DEBUG_UTILS_EXTENSION_NAME) to the extensions vector.
###             This extension is essential for debugging and enables the use of features such as message callbacks.
###             *return extensions;* : Returns the final list of extensions, including the debug utility extension if enableValidationLayers is true.
            Key Points:
                      Dynamic Extension Handling:
                                                 The function dynamically adjusts the required extensions based on whether validation layers are enabled. This ensures the application only includes the extensions it needs.
                     VK_EXT_DEBUG_UTILS_EXTENSION_NAME:
                                                      This macro expands to "VK_EXT_debug_utils", the name of the Vulkan extension for debug utilities.
                    Integration with Vulkan:
                                            The returned extensions vector is usually passed to the VkInstanceCreateInfo structure when creating a Vulkan instance. so let's do that next
#### ==we go to CreateInstance()==
# inside the CreateInstance()

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

This snippet of code configures the VkInstanceCreateInfo structure during the setup of a Vulkan instance. Here’s a breakdown of its functionality and purpose:

#Explanation:
### 1.  *auto extensions = getRequiredExtensions();*: Calls the getRequiredExtensions() function to retrieve a list of required extensions, including debug utilities if validation layers are enabled.
### 2. *createInfo.enabledExtensionCount:* Sets the number of extensions that will be enabled for the Vulkan instance.The value is cast to uint32_t because Vulkan uses fixed-size types.
### 3. *createInfo.ppEnabledExtensionNames:* Points to the array of extension names (extensions.data()), which is managed by the std::vector returned from getRequiredExtensions().
# Validation Layers:
### 4. *if (enableValidationLayers):* Checks whether validation layers are enabled (useful for debugging during development).
### 5. *createInfo.enabledLayerCount:* If validation layers are enabled, sets the count to the size of the validationlayers vector.
###                                   Validation layers are typically pre-defined as a std::vector<const char*>, listing names like "VK_LAYER_KHRONOS_validation".
### 6. *createInfo.ppEnabledLayerNames:* Sets the pointer to the array of validation layer names (validationlayers.data()).
### 7. else { createInfo.enabledLayerCount = 0; }: If validation layers are not enabled, sets the layer count to 0, disabling all validation layers.
        
Key Points:
1. Extensibility:The function dynamically configures both extensions and validation layers, ensuring flexibility for different development and deployment environments.
2. Validation Layers:These are critical for debugging Vulkan applications but should be disabled in production for performance reasons.
3. Safety with std::vector: Using std::vector to manage extension and layer lists ensures memory safety and avoids manual allocation.
Example of Validation Layers:
The validationlayers vector might look like this:

# Copy code
          const std::vector<const char*> validationlayers = {
                   "VK_LAYER_KHRONOS_validation"
                };
###This layer enables a comprehensive set of validation checks provided by the Vulkan SDK.

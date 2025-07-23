==debug callback() function==
## debug callback() is a function that captures validation layer messages during application execution
## Here's a breakdown of it's component and purpose
# Code Example:
 
	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

# 1. Funciton Definition
#Copy Code:
          static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(...)
#  VKAPI_ATTR and VKAPI_CALL:* Are Macros that ensure the function uses the correct calling convention for Vulkan, typically __stdcall on Windows.
#  static: Limits the function's scope to the current translation unit.
#  VkBool32: A Vulkan-specific boolean type (uint32_t), with VK_FALSE (0) and VK_TRUE (1) as possible value

# 2.  Parameters:
# messageSeverity: Indicates the severity of the message (e.g., error, warning, info, or verbose). Values are from the VkDebugUtilsMessageSeverityFlagBitsEXT enumeration.
# ProtoType:
            
            typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
                  } VkDebugUtilsMessageSeverityFlagBitsEXT;
                  
# messageType: Specifies the type of message (e.g., general, validation, or performance). Values are from the VkDebugUtilsMessageTypeFlagsEXT enumeration.
# ProtoType:
     
            typedef VkFlags VkDebugUtilsMessageTypeFlagsEXT;
ProtoType:
            typedef uint32_t VkFlags;
            
# pCallbackData: A pointer to a VkDebugUtilsMessengerCallbackDataEXT structure, which contains details about the validation message, including:
#               pMessage: A null-terminated string with the debug message.
#               Other fields with information about the source and objects involved in the message.
#ProtoType:

                typedef struct VkDebugUtilsMessengerCallbackDataEXT {
                           VkStructureType                              sType;
                           const void*                                  pNext;
                           VkDebugUtilsMessengerCallbackDataFlagsEXT    flags;
                           const char*                                  pMessageIdName;
                           int32_t                                      messageIdNumber;
                           const char*                                  pMessage;
                           uint32_t                                     queueLabelCount;
                           const VkDebugUtilsLabelEXT*                  pQueueLabels;
                           uint32_t                                     cmdBufLabelCount;
                           const VkDebugUtilsLabelEXT*                  pCmdBufLabels;
                           uint32_t                                     objectCount;
                           const VkDebugUtilsObjectNameInfoEXT*         pObjects;
                           } VkDebugUtilsMessengerCallbackDataEXT;
                           
# pUserData: A user-defined pointer for passing your own data to the callback.

# 3. Functionality:
## Copy Code :

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

#          Outputs the validation message to std::cerr.
#          Typically used during debugging to log issues flagged by validation layers.
# 4. Return Value:
# return VK_FALSE; Indicates that Vulkan should not abort or interrupt operations after this callback.


## vulkan platform specific calling convetion documentation states
 Platform-specific calling convention macros.
 
  Platforms should define these so that Vulkan clients call Vulkan commands
  with the same calling conventions that the Vulkan implementation expects.
 
  VKAPI_ATTR - Placed before the return type in function declarations.
               Useful for C++11 and GCC/Clang-style function attribute syntax.
  VKAPI_CALL - Placed after the return type in function declarations.
               Useful for MSVC-style calling convention syntax.
  VKAPI_PTR  - Placed between the '(' and '*' in function pointer types.
# ProtoType: 
            Function declaration:  VKAPI_ATTR void VKAPI_CALL vkCommand(void);
            Function pointer type: typedef void (VKAPI_PTR *PFN_vkCommand)(void);
 
### ProtoType:
             #if defined(_WIN32)
                    // On Windows, Vulkan commands use the stdcall convention
                 #define VKAPI_ATTR
                 #define VKAPI_CALL __stdcall
                #define VKAPI_PTR  VKAPI_CALL
                .....



# All that remain is tell vulkan  about the callback() function by using a vulkan handle
# that needs to be explicitly created and destroyed such a callback is part of a debug Messenger
 ==we move to IkDevice.hpp file== in the Private section
 # Private Section
 - [] we declare a new function called 
# Code Example:
               void setupDebugMessenger();
               
then we move to the cpp file 
 ==we move to IkDevice.cpp file==
 -[] we then declare the setupDebugMessenger() inside the constructor
# Code Example:

              IkeDevice::Engine(IkeWindow& window) : window(){
                  createInstance();
                  setupDebuMessenger();
                  }
- [] Now we can define the setupDebugMessenger() where we need to
##  Check if the validation layers are enabled if so return
# CodeExample:

             void IkeDeviceEngine::setupDebugMessenger(){
                if(!enableValidationlayers) return;
                
             }


- [] We need to fill in a struct with details about the messenger and it's callback
#    we do that by referencing VkDebugUtilsMessengerCreateInfoEXT struct
## ProtoType:
            
            
                    typedef struct VkDebugUtilsMessengerCreateInfoEXT {
                         VkStructureType                         sType;
                         const void*                             pNext;
                         VkDebugUtilsMessengerCreateFlagsEXT     flags;
                         VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
                         VkDebugUtilsMessageTypeFlagsEXT         messageType;
                         PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
                         void*                                   pUserData;
                         }VkDebugUtilsMessengerCreateInfoExt;

-[] we need to first declare the functin we need for the operation in the hpp file
==ikDevice.hpp== 
# Private Section
we declare the function
# Code Example:

               void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
- [] then we define it in the cpp file
==ikDevice.cpp==
Code Example:

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
# like every vulkan structure we filled we define  a void return type function to hold the referenced struct as it's argument
This function, `IkeDeviceEngine::populateDebugMessengerCreateInfo`, is a helper function designed to set up a `VkDebugUtilsMessengerCreateInfoEXT` structure. This structure is used to configure Vulkan's debug messenger, which enables validation layer messages to be captured and processed.

---

### **Breakdown of the Code**

1. **Function Signature**:
   ```cpp
   
          void IkeDeviceEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
   ```
   - **`IkeDeviceEngine::`**: Indicates this function is a member of the `IkeDeviceEngine` class.
   - **`VkDebugUtilsMessengerCreateInfoEXT& createInfo`**: The function takes a reference to a `VkDebugUtilsMessengerCreateInfoEXT` structure, which will be populated.

2. **`createInfo = {};`**:
   - Initializes the `createInfo` structure to default values.

3. **`createInfo.sType`**:
   ```cpp
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   ```
   - Sets the structure type to `VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT`, identifying it as a debug messenger create info structure.

4. **`createInfo.messageSeverity`**:
   ```cpp
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   ```
## ProtoType:
          
                    typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
                      } VkDebugUtilsMessageSeverityFlagBitsEXT;
                      
     Notice that we specified this VkDebugUtilsMessengerSeverityFlagBitsEXT in debugCallback() earlier                
   - it Specifies the severity levels of messages the debug messenger will handle:
     - **`VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT`**: Warnings (potential issues).
     - **`VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT`**: Errors (serious issues).
   - Other options (e.g., `VERBOSE`) can be added as needed.

5. **`createInfo.messageType`**:
   ```cpp
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   ```

## ProtoType:
             
             typedef enum VkDebugUtilsMessageTypeFlagBitsEXT {
                  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,
                  VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT = 0x00000008,
                  VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
                  } VkDebugUtilsMessageTypeFlagBitsEXT;

    Notice the same as this too we specified inside debugCallback() earlier
   - Specifies the types of messages the debug messenger will handle:
     - **`GENERAL`**: Informational messages unrelated to validation or performance.
     - **`VALIDATION`**: Messages related to Vulkan API misuse or issues.
     - **`PERFORMANCE`**: Warnings about potential performance bottlenecks.

6. **`createInfo.pfnUserCallback`**:
   ```cpp
   createInfo.pfnUserCallback = debugCallback;
   ```
   - Specifies the user-defined callback function (`debugCallback`) to handle messages. This function should conform to the `VkDebugUtilsMessengerCallbackEXT` prototype.


7. **`createInfo.pUserData`**:
   ```cpp
   createInfo.pUserData = nullptr; // optional
   ```
   - An optional pointer for user-defined data that will be passed to the callback. In this case, it is set to `nullptr`.

---

### **Purpose**

- This function is a utility to reduce redundancy and ensure consistency when setting up the `VkDebugUtilsMessengerCreateInfoEXT` structure.
- It can be reused wherever a debug messenger is created, streamlining the process.

---

### **Usage Example**
Here’s how this function might be used in a Vulkan application:

```cpp
VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
IkeDeviceEngine deviceEngine;
deviceEngine.populateDebugMessengerCreateInfo(debugCreateInfo);

// When creating the Vulkan instance
VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pNext = &debugCreateInfo; // Attach debug messenger info
```

---

### **Key Benefits**
1. **Encapsulation**:
   - Keeps debug messenger setup logic within a single function, making the codebase cleaner and easier to maintain.
   
2. **Reusability**:
   - This function can be reused across different parts of the application or project.

3. **Flexibility**:
   - By modifying the `populateDebugMessengerCreateInfo` function, you can adjust the debug messenger behavior globally without needing to update multiple locations in the code.

## This design is a clean, reusable approach to handling Vulkan debug messenger configuration.
##   The struct should be passed to the `vkCreateDebugUtilsMessengerEXT`function to create the *VkDebugUtilsMessengerEXT* object



## now we have to call the setupDebugFunction again but this time it will have 
1. a struct ,
2. a function 
3. and an if condition that will check if the CreateDebugUtilsMessengeEXT is successfull Notice that the function itself is used to take in the structs which is 
    we check if it has the 
                           instance, the struct VkDebugUtilsMessengerCreateInfoEXT,
                           the struct *pAllocator set to null,the VkDebugUtilsMessengerEXT Handle

	void IkeDeviceEngine::setupDebugMessenger() {
		if (!enableValidationlayers) return;


		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
			&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

after we have finished we must not forget to go to he destructor and destroy the DestroyDebugUtilsMessengerEXT()
##ikeDeviceEngine.cpp 

           if(enableValidationlayers){
              DestroyDebugUtilsMessengerEXT(instance,debugMessenger, nullptr);
           }

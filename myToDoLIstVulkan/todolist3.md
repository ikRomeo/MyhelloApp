==ikDeviceEngine==

### Creating the Instance
## we start off by creating two files .cpp and .hpp files

#ikDeviceEngine.hpp
- []we start with header guards 
     #inside the header guards 
- [] we inlcude the ikWindow.hpp header so that we can access the window
- [] we use the namespace Ike block
     #inside the namespace Ike block
- [] we add the IkWindow.hpp header
- [] we create a IkeDeviceEngine class
     #inside the class IkeDeviceEngine
### Public section;
- [] we declare the constructor with IkeWindow reference as an argument 
 - [] we declare the destructor with no argument
      # we create and assign delete to the copy and move constructors
- [] we assign delete to the copy constructor and
- [] we assign delete to the copy assignment operator
- [] we assign delete to the move constructor
- [] we assign delete to the move assignment operator
    
### Private section:

- [] we declare the createInstanc() with void return type
 - we instanciate the  VkInstance handler
 - we instanciate and reference  the IkeWindow handler
#ikDeviceEngine.cpp

- [] we include ikDeviceEngine.hpp header
- [] we start the ikE namespace block
   #inside the ikE namespace block
- [] we initialized the IkeDeviceEngine constructor which takes a reference of IkeWindow as an argument
- [] inside the constructor we declare again the createInstance()
- [] we initialize the destructor()
- [] inside the destructor we call the vKDestroyInstance() that takes the instance and nullptr
- [] then we call glfwTerminate()

-[] we define the the createInstance()
  * we Objectify VkapplicationInfo{} and fill up the struct members
## Prototype

                ---------------------------------------
#####                   typedef struct VkApplicationInfo {
#####                      VkStructureType    sType;
#####                      const void*        pNext;
#####                      const char*        pApplicationName;
#####                       uint32_t           applicationVersion;
#####                       const char*        pEngineName;
#####                       uint32_t           engineVersion;
#####                       uint32_t           apiVersion;
#####                        } VkApplicationInfo;
                ------------------------------------------
                
# Code Example:

                      VkApplicationInfo appInfo = {};
                      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                      appInfo.pApplicationName = "BronixEngine App";
                      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                      appInfo.pEngineName = "No Engine";
                      appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
                      appInfo.apiVersion = VK_API_VERSION_1_0;
                  
       
  * we Objectify VkInstanceCreateInfo{} where we reference the VkapplicationInfo Object 
## Prototype

             -----------------------------------------------------------
             
#####                      typedef struct VkInstanceCreateInfo {
#####                          VkStructureType             sType;
#####                          const void*                 pNext;
#####                          VkInstanceCreateFlags       flags;
#####                          const VkApplicationInfo*    pApplicationInfo;
#####                          uint32_t                    enabledLayerCount;
#####                         const char* const*          ppEnabledLayerNames;
#####                          uint32_t                    enabledExtensionCount;
#####                         const char* const*          ppEnabledExtensionNames;
#####                       } VkInstanceCreateInfo; -
             --------------------------------------------------------------
 NOTICE: that the ppEnabledExtensionNames is a variable witn a  const modifier pointer 
         and a return type char which is also a pointer with a const modifier
         
# CodeExample:

              VkInstanceCreateInfo createInfo = {};
              createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
              createInfo.pApplicationInfo = &appInfo;

              uint32_t glfwExtensionCount = 0;
              const char** glfwExtensions;
              glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

              createInfo.enabledExtensionCount = glfwExtensionCount;
              createInfo.ppEnabledExtensionNames = glfwExtensions;
              
              createInfo.enabledLayerCount = 0;

-[] we initialize the glfw extension to interface with vulkan by
  * we assign 0 to glfwExtentionCount of uint32_t return type
  * we create a variable glfwExtensions which is a pointer to pointer of type char with a const modifier
    that we will use to access *ppEnabledExtensionNames* which is also a pointer to a  pointer
  * we assign the glfwGetRequiredInstanceExtensions() funtion with address of glfwExtensionCount as it's argument
# Prototype
#####          GLFWAPI const char** glfwGetRequiredInstanceExtension(uint32_t count);
  * we assign the glfwExtensionCount to the createInfo.enabledExtensionCount{} struct
  * we assign the glfwExtensions to the createInfo.ppEnabledExtensionNames 
  * we assign 0 to the createInfo.enabledLayerCount 
  * we call the vkCreateInstanced() which takes 
     1. a reference to CreateInfo struct,
     2. a pointer to custom allocator callbacks, which is nullptr for now
     3. a pointer to the variable that stores the handle to the new Object 
         which is the reference to instance 
    which we wrapped into an if statement if not equalt to VK_SUCCESS throw an std::runtime_error
## Code Example:

                if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
	                       throw std::runtime_error("failed to create instance");
                            }
NOTE: 
        we will be need to include<stdexcept> for runtime error and  # include <iostream> to print out the error

        == we can compile the program now to see that the instance is created ==
                    == checking for extensions ==
        ## we want to retrieve a list of supported Extensions which is an optional functionality
           before creating an instance 
           the vkEnumerateInstanceExtensionProperties() takes 
           1. a pointer to a variable that stores a number of extensions 
           2. an array of VkExtensionProperties to store details of the extensions.
           3. an optional first parameter that allows us to filter extensions by a 
              specific validation layer, which we will ignore for now

#Prototype
#####            VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(
#####                           const char*                                 pLayerName,
#####                           uint32_t*                                   pPropertyCount,
#####                           VkExtensionProperties*                      pProperties);

##          To allocate an array to hold the extension details 
##           we first need to know how many there are by getting the number of extensions
            
## we will first go to ikDeviceEngine.hpp file 
    * and declare the hasglfwGetRequiredExtenstion() of void returnType
     #NOTE: the hasglfwGetRequiredExtension() is ours not a glfw object. inside the function we access vkEnumerateInstanceExtensionProperties
## back inside our ikDeviceEngine.cpp file
- [] we define the hasglfwGetRequiredExtension() of void returnType 
       # inside the hasglfwGetRequiredExtention()
- [] we initialize a uint32_t variable extensionCount and assign 0 to it 
- [] we call vkEnumerateInstanceExtensionProperties with reference to extensionCount and the other two parameters as null pointers 
    Example:
            vkEnumerateInstanceExtensionProperties(nullptr,&extensionCount, nullptr);
- [] we allocate an array to hold the extension details .we need to include<vector> header file
    Example:
            std::vector<VKExtensionProperties> extensions(extensionCount);
- [] finally we query the extension details
    Example:
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
      # NOTE: each vkExtensionProperties struct contains the name and version of an extension. we can list them with a simple for-loop
- [] we first std::cout the "available extensions:\n";
- [] we create a for loop to loop through the extensions 
     Example:
             for(const auto & extension : extensions){
        - [] we std::cout << "\t " << extension.extensionName << "\n"
            

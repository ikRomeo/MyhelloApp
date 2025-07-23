==PickPhysicalDevice==

after we have initialized the VkInstance handle we will need to look for and select a graphics card in the
system meaning the (operating system) that supports the features we will be needing
* we can select any number of graphics cards and use them simultaneously
  but we will stick to first graphics cards that suit our need

# ikDeviceEngine.hpp 
### Private sectcion
  -[] we will define a function pickPhysicalDevice of void return type
# Code Example:
              
              void pickPhysicalDevice();
   -[] along with other helper function we will Objectify VkPhysicalDevice and initalized it vknull pointer handle
# Code Example:
               
               VkPhysicialDevice physicalDevice = VK_NULL_HANDLE;
the graphics card that we'll end up selecting will be stored in a VkPhysicalDevice handle that is added as a new class member
then we move to
# ikDeviceEngine.cpp
   -[] we will add the pickPhysicalDevice() to the constructor
# Code Example:
               
               pickPhysicalDevice();
  -[] we will now declare the function
# Code Example:
               
               void IkeDevice::pickPhysicalDevice(){}
## inside the function
   -[] listing the graphics card is very similar to listing extensions and start with querying just the number

# Code Example:

               uint32_t deviceCount = 0;
               vkEnumeratePhysicalDevices(intance,&deviceCount, nullptr);
   -[] if there is 0 devices with Vulkan Support then there is no need to go further
# Code Example:

               if(deviceCount == 0){
               throw std::runtime_error("failed to find GPU's with Vulkan support");
               }
   -[] Otherwise we can now allocate an array to hold all of the VKPhysicalDevice handles
# Code Example:
            
               std::vector<VKPhysicalDevice> devices(deviceCount);
               vkEnumeratePhysicalDevices(instance,&deviceCount, devices.data());

Now we need to evaluate each of them and check if they are suitable for operations we want to perform.
because not all graphics cards are created equal for thet we will have to introduce a new funciton

# ikDeviceEngine.hpp file 
  -[] in the helper function area , we define a bool return type with the VKPhysicalDevice as argument 
## Code Example:
                
                bool isDeviceSuitable(VkPhysicalDevice device);

we will then declare the function in ikDeviceEngine.cpp file

# ikDeviceEngine.cpp file
 -[] we will define the function with a return type of true for Now
# Code Example:
             
              bool IkeDeviceEngine::isDeviceSuitable(VKPhysicalDevice device){
                      return true;
              }
 Now we can proceed inside the pickPhysicalDevice() to check if any physical devices meet the requirements
 that we will add to the function
##inside the void IkeDevice::isDeviceSuitable()
 We will now check if any of the physical devices meets the requirements 
          
           
           for (const auto device : devices){
                if(isDeviceSuitable(device)){
                physicalDevice = devices;
                break;
                }
           }
           if (pysicalDevice == VK_NULL_HANDLE){
               throw std::runtime_error("failed to find a suitable GPU!");
           }
The next sections we will introduce the first requirements that we'll check for in the isDeviceSuitable() function
As we'll start using more 'Vulkan features' later and we will also extend this isDeviceSuitable() function to include more checks

==Base device suitability checks==
To evaluate the suitability of a device we can start by querying for some details.
Basic device properties like : The name,type and supported Vulkan version can be queried using vkGetPhysicalDeviceProperties

# ikDeviceEngine.hpp file
we will first define the function in the 
# Public Section
    -[] we will define the VKPhysicalDeviceProperties handle
    
# Code Example:

               VkPhysicalDeviceproperties deviceProperties;

we will then proceed in using it inside pickPhysicalDevice() in 
# ikDeviceEngine.cpp file

               vkGetPhysicalDeviceProperties(device, &deviceProperties);
we will then print out the device properties of features 
# Code Example: 

                   std::cout << "physical device" << properties.deviceName << std::endl;
                   
==Queue families== 
Every operation in Vulkan, from drawing,uploading textures requires commands to be submitted to the Queue 
There are different Queue Families and each Family of queues allows only a subset of commands.
Example there could be a queue family that only allows processing of compute commands or one that only allows memory tranfer related commands 
We need to check which queue families are supported by the device and which one of these supports the commands that we want to use for that purpose 
we'll add a new function 'findQueueFamilies' that looks for all the queue families we need.right now we only need the one that supports graphics commands

because we will be needing multiple queues from the queuefamilies we will be using a struct to bundle the queue members we need
we need to go to 
## ikDeviceEngine.hpp file
# Public Seciton:
The idea is to 
define a struct QueueFamilyIndices with a member graphicsFamily of uint32_t type
we use it as a return type of the findQueueFamilies() function that has VkPhysicalDevice HANDLE as an argument

- [] we will create a struct to hold the queue members we need  for now we need the one for graphics but we will also add the presentFamily
     The graphicsFamily supports graphics operations i.e rendering
     The presentFamily supports presentint images to a surface
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
                      };
Note: that we will need sometype of way of indicating whether a particular queue family was found
then we will use the QueueFamilyIndices as a return type of a function that has the VkPhysicalDevice as an argument in the private section

##Note that this code is for commandpool
### Code Example
                
                QueueFamilyIndices findPysicalDeviceFamilies(){return findQueueFamilies(physicalDevice);};
# Private Section:
 among other helper functions
 - [] we use the struct QueueFamilyIndices as a return type to define a function findQueueFamilies that take VkPhysicalDevice as an argument
### Code Example
                
                QueueFamilyIndices  findQueueFamilies(VkPhysicalDevice device);

we then move to 
## ikDeviceEngine.cpp file where we are goint to
- [] declare the findQueueFamilies() and its functionality
### Code Example
Notice how we use the namespace::class::struct class::function

                   ikE::IkeDeviceEngine::QueueFamilyIndicices IkeDeviceEngine::findeQueueFamilies(VkPhysicalDevices device){}
                   
##inside the findQueueFamilies function
  inside the function we will 
  -[] we will create a variable of type uint32_t to hold the number of queuefamilies we need
                        
                        uint32_t queueFamilyCount = 0;
                        
   we will then use a 3way process to query and retrieve the data with the help of a function
   vkGetPhysicalQueueFamilyProperties(has the physicalDevice, the number of elements,and the VkQueueFamilyProperties);
   Note: the VkQueueFamilyProperties is a struct that has {the VkQueueFlags that is a uint32_t typedefed,uint32_t queue count,
                                                                uint32_t timestampValidBits and a struct VKExtend3D{that has uint32_t width, 
                                                                                                                     uint32_t height,uint32_t depth}
                       
step1: we discover the number of queue families (pre-query check)
                        vkGetPhysicalQueueFamilyProperties(device,&queueFamilycount,nullptr);
step2: we allocate space and query queue families (Request data via vector)
                         std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                         vkGetPhysicalQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
step3: Evaluate and process the retrieved queue families(Retrieved Data)
	       
           int i = 0;
	       for (const auto& queueFamily : queueFamilies) {
		       if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			        indices.graphicsFamily = i;
			        indices.graphicsFamilyHasValue = true;
		          }

		        i++;
	             }

Now that we have the fancy queue family lookup function we can use it as a check in the isDeviceSuitable()

                   bool IkeDeviceEngine::isDeviceSuitable(VKPhysicalDevice device){
                      QueueFamilyIndices = findQueueFamilies(device);
                      return true;
                       }

we can now also use this for an early exit from findQueueFamilies
                      
                      if(indices.isComplete()){
                      break;
                      }

thats all we need for now to find the right physical device The next step is to create a logical device

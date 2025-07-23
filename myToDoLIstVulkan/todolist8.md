After selecting physical device to use we now need to set up a logical device to interface with it.
logical device creation is similar to the instance creation
we also need to specify which queues to use now that we have queried queue families earlier on
Note: that queues are essential for parallel execution of graphics, compute and transfer tasks
       on the GPU
Vulkan defines queues Family and each queue family supports different types of operations

QueueTypes:
          GraphicsQueue:Renders images,handles vertex processing, fragment shading e.t.c
          Examples:
                   Rendering 3D objects,UI,post processing
          ComputeQueue: Runs general-purpose GPU computations,(GPGPU) without graphics operations
          Examples: 
                   Physics Simulations, AI inference, ray Tracing
         TransferQueue: Handles memory transfers without blocking rendering and compute operations
         Examples: 
                  Uploading textures, streaming data
         SparseQueue: Manages sparse resources(not commonly used)
         Examples:
                  Virtual textures,sparse voxel octrees
         
we need to create a function void logical device in the ikeDevice hpp file as a new class member
##ikeDevice.hpp file
#Private Section        

        void createlogicalDevice();
The creation of a logical device involves specifying a bunch of details in structs again.of which the
first one will be VkDeviceQueueCreateInfo. so under the helper function still in ikeDevice.hpp file

then we create the handle which is VkDevice 
        
        VkDevice device_
  then we create the getter function
        
        VkDevice device(){return device_;};

  then we also create another getter function that is responsible in getting the QueueFamilyIndices
  
        QueueFamilyIndices findQueueFamiies(VkPhysicaldevice device);
then we need to create the handle for both graphicsQueue and presentQueue to interface with the logical device
        
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
the we need to create the getter functions for both handles
        
        VkQueue graphicsQueue(){ return graphicsQueue_;};
        VkQueue presentQueue(){ return presentQueue_;};
then we define it in the cpp file
##ikeDevice.cpp file
we initalize the CreateLogicalDevice in the constructor
           
           createLogicalDevice();
           
we initailize the void logicalDevice() with the QueueFamilyIndices

        void IkeDevice::createLogicalDevice(){
             VkDeviceFamilyIndices  indices = findQueuFamilies(physicalDevice);
             
vulkan requires us to create queues to handle different task i.e(graphics,compute,transfer, e.t.c)
and we will need multiple queue creation structures(VkDeviceQueueCreateInfo), so we need to use
std::vector which allows for dynamic storage
then we also need std::set because w do not need duplicates so that we can ensure(graphicsFamily which is 
index for graphics processing queue that handles rendering and presentFamily which is index for presenting images to the screen)
are not the same also we need to add #include set to the header
             
             std::vector<VkDeviceQueueCreateInfo> queueCreateInfos
             std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily,indices.presentFamily};

The currently available drivers will only allow you to create a small number of queues for each queueFamily and we really
don't need morethan one because we can create all of the command buffers on multiple threads and then submit them all at once
on the main thread with single low-overhead 
so by prioritising which queue gets more execution time when multiple queues are executed 

             float queuePriority = 1.0f
the 1.0f is the maximum priority value ,0.5f is midium priority value, 0.0f minumum priority value, and vulkan priority values ranges from 0.0f to 1.0f
Example:
       renderqueue is maximum priority 1.0f
       computequeu is midium priority 0.5f or 0.7f
       Background queue is lowest priority 0.0f or 0.3f
       
Priorities is useful because it ensures that critical tasks get executed before the background tasks,it helps optimise GPU resources allocation for 
better performance and can reduce frame latency by prioritising graphics over compute workload

This variable is later passed into VkDeviceQueueCreateInfo
to access each of the queues we will need a for-loop that will iterate over each Queue Family index stored in uniqueQueueFamilies and create a 
VkDeviceQueueCreateInfo structure for it 
            
            for(uint32_t queueFamily : uniqueQueueFamilies){
                   VkDeviceQueueCreateInfo queueCreateinfo = {};
                   queueCreateInfo.sType = Vk_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                   queueCreateinfo.queueFamilyIndex = queueFamily;  // this is an integer that tell vulkan which queuefamily this queue belongs
                   queueCreateinfo.queueCount = 1; // specifies that we want 1 queue from the selected queue family it can also be multiple queues per queue family
                   queueCreateinfo.pQueuePrioriies = &queuePriority; //this is a pointer to a float array that defines the priority
                   queueCreateInfos.push_back(queueCreateInfo); //adds queueCreateInfo to the queueCreateInfos vector
                   
            }
Next is to specify the set of device features that we'll be using, this are the features we queried support for with vkGetPhysicalDeviceFeatures
            
            VkPhysicalDeviceFeatures deviceFeatures = {};
            deviceFeatures.samplerAnisotropy = VK_TRUE; // it improves texture clarity when viewed at an angle.if not supported by the gpu then vulkan will return false 
then we create struct for the logical device

            VkDeviceCreateInfo createInfo = {};// creates a vulkan logical device configuration
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());// passes queue configuration for the logical device
            createInfo.pQueueCreateInfos = queueCreateInfos.data() // pointer to the first queue configuration in the vector 
            createInfo.pEnabledFeatures = &deviceFeatures; //passes a pointer to enabled GPU features (e.g Anisotropic filtering)
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionsNames = deviceExtensions.data();

then we instantiate  the logical device with a call with parameters(physical device to interface with, the queue and usage info we just specified, 
                                                                     the optional allocation callbacks pointer and a pointer to a variable to store the 
                                                                     logical device handle in)
            
            if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS){
               throw std::runtime_error("failed to create logical device");
            }
            
Note: because we passed device extension when creating logical device we need to go back to ikeDeviceEngine.hpp file
## ikeDeviceEngine.hpp file
and here we need to add

                        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
This is  enables swapchain support which mangages the images and buffers .and passed to VkDeviceCreateInfo when creating the logical device



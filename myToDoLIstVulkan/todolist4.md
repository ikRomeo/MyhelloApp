#                   ==Validation layers==
          Are optional components that hook into vulkan function calls to apply additional operations
 
#         Common Operations:
              * Checking of values of parameters list against the Specification to detect misuse
              * Tracking Creation and destruction of Objects to find resource leaks
              * Checking thread safety by tracking the threads that calls originate from
              * logging every call and it's prameters list to the standard output
              * Tracking vulkan calls for profiling and replaying it  
  

  ToDo start
    because the validation layer in bundled in an sdk layer 
    known as "Vk_LAYER_KHRONOS"
###### inside ikDevice.hpp file
==Private Section== of *class IkeDeviceEngine*
   - [] we will create a dynamic array to hold the VK_LAYER_KHRONOS_validation string
      ###Nete: we will need to include vector and string headers to use vector array and string types

      
       Example:
              const std::vector<const char*> validationlayers = {"VK_LAYER_KHRONOS_validation"};
              ^         ^              ^              ^                           ^ 
              |         |              |              |                           |
         we make    we use a    we typecast   variable name       it is a string the reason
         the array  a dynamic   kit to char                        we have to typecast it to char
         immutable   array       and made it's
                                elements 
                                immutable                 
==Public Section==  of *class IkeDeviceEnging*
     - [] we need to add the macro condition to check if the validation layer is enabled in debug mode 

         
       Example: 
                 #ifdef NDEBUG
                            const bool enableValidationlayers = false;
                #else
                            const bool enablevalidationlayers = true;
                #endif

                
==Private Section== of *class IkeDeviceEngine*  
## Note: inside the private section we have *methods,handlers and arrays as members of the class
## we will declare a new function *checkValidationLayerSupport* of type bool  to see if all the requested layers are available
 
        Example: 
                bool checkValidationLayerSupport(); 

###### inside ikDevice.cpp file
- [] we will define the checkvalidationlayersupport() function
       Example:
              bool IkeDeviceEngine::checkValidationLayerSupport(){}
    ## inside the IkeDeviceEngine::checkValidationLayerSupport() function
## we will list the availabe layers using Two way process to query the elements
          where we first use a variable to know how many elements are in the array
          then we we will query the elements of the array via vkEnumerateInstanceLayerProperties()
#### Firstcall of two way process
# 1. - [] we Determine the the number of Available layers by 
          we create a varaible of layerCount of type uint32 and set it to 0 it's purpose is to serve
          as a way to access the index of the VKLayerProperties via the pPropertyCount which is of the
          same type but a pointer
#        Example:

                     uint32_t layerCount =   0; 
                       ^            ^        ^
                       |            |        |
                      
                a whole number   variable  initializer
                no negative      name
                number and 
                compatible 
                in all  
                machine
                
## similar to vkEnumerateInstanceExtension we will call vkEnumerateInstanceLayerProperties() using the two way process
       * [] then we will call the  number of elements of layerProperties
## Prototye 
      ------
          VKAPI_ATTR VKResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VKLayerProperties* pPrpoerties);
              ^           ^           ^            ^                                  ^                            ^
              |           |           |            |                                  |                            |
            macro        enum        macro        functionName                  pointer to number          a struct that holds the features
                                                                                of elements                or properties the validation layer 
                                                                                                           extension should have 
                                                                                                           Syntax:
                                                                                                                 typedef struct VKLayerProperties{
                                                                                                                     char layerName[VK_MAX_EXTENSION_NAME_SIZE];
                                                                                                                     uint32_t specVersion;
                                                                                                                     uint32_t implementationVersion;
                                                                                                                     char  decsription[VK_MAX_DESCRIPTION_SIZE];
                                                                                                                               }VKLayerProperties;
                                                                                                                 
      ------
 

# Code £xample:

                   vkEnumerateInstanceLayerProperties(&layerCount ,   nullptr);                     
                        ^                              ^                  ^
                        |                              |                  |
                  function Name            we take the address       pProperties which is a pointer to a struct is set to nullptr
                                           of the layerCount         because we are only interested in the number of layer not their details for now
                                           to access -->              
                                           uint32_t* pointer 
                                           but on return from
                                           this call it will be 
                                           set to 0 or contain 
                                           number of 
                                           availablelayers
#### SecondCall of two way process
# 2. - [] we will Allocate memory for the elements of the Available layers by first using dynamic array via std::vector
          notice that we did not typecast it but instead we access the elements of the struct VKLayerProperties
          which we Objectified with our own variabe which we created called availableLayers() with the layerCount as an argument
          notice that we did not use the reference operator because we want its value which is the 4 member elements of VKLayerProperties
          
#      Code Example

                  std::vector<VKLayerProperties> availablelayers(layerCount);
# 3. - [] Now we can Retrieve the layer Properties
          we will now call again the vkEnumerateInstanceLayerProperties with the help of std::vector method data()
          Note: data() is a vector pointer  that points to the first element of the vector array in this case is  
          VKLayerProperties is the first element of the array 
#     Code Example
                 
                  vkEnumerateInstanceLayerProperties(&layerCount, availablelayers.data());
 

     
# 4. - [] we check for Required validation meaning we will check if all 
#         layers in validation layers exist in the available layers list
        we will include <cstring> for this
        we will need to use for loop to iterate through the vector elements
For     OUTER loop: check if each required layer is available in ValidationLayers 
#       Code Example:
        
                    for(const char* layerName : validationLayers){ 
                    bool layerFound = false;
                     ... 
                             }

## we run a loop where we check inside the ValidationLayers with the help of the immutable const char pointer
## which is the best used case of accessing individual string elements better than std::string 
## Notice that validationLayers is also a const char pointer which holds the vulkan sdk 
## VK_LAYER_KHRONOS_validation extensions which is a string we typecasted in to char pointer
## then we created a flag of bool return type that we set to false we will set it back to true after the comparison
For  INNER loop: we search for the layer in the availble layer
#    Code Example:

                  for(const auto& layerProperties: availableLayers){
                       if(){}...}
                  
## rather than making a copy of each element in availbleLayers we use reference operator instead
## Objectified by our made up variable layerProperties
## the const help us to make it immutable meaning the loop operation can read the layerProperties
## but it can not modify it 
## auto automatically deduce the type fo layerProperties from the vector availableLayers making
## the code more coincise and maintainable
##NOTE: we will have to include<cstring> for strcmp
For   INNER loop IF-Condition(innerloop):
#  Code Example:

                if(strcmp(layerName, layerProperties.layerName) == 0){
                    layerFound = true;
                    break;
                    
                }
## the strcmp() function which is a standard C function that compares two C-style strings(null terminated character arrays)
## it returns '0' if the strings are identical and '1' if they are not
## in this case we compare the required *layerName* which is a const char pointer that indexis the *validationLayers* in the  for-loop range
## with the *layerProperties* which is the index of *availableLayers*  in the for-loop range
## a struct stored in dynamic array container via std::vector<VKLayerProperties> that we
## Objectified as *availablelayers(LayerCount)*  so the layerProperties.layerName means we are accessing the layerName via *layerProperties*
## to see if they are both equal to 0 meaning succesfull
## if the operation is  succesfull we assign true to layerFound meaning the flag become true then we break out of the **IF-Condition**
## NOTE: layerProperties Represents a single VKLayerProperties Object  via availableLayers that holds layerCount as an argument
  INNER loop if condition(outsideloop);
# Code Example:
              
                if(!layerFound){
                  return false;
                }
    
## but if the layer is not found after we have run the INNER and OUTER for-loop operation we will return false and exit the INNER loop  
## then we can return true from the checkValidationLayerSupport() which has bool return type
# 5.  - [] we can now use the function in createInstance()function
##  at the top of the *appInfo* struct we will add this lines of code 
#  Code Example:
             
               if(enableValidationlayers && !checkValidationLayerSupport()){
                 throw std::runtime_error("validation layers requested, but not available!");
               }

## this *IF-Conditon* checks two things:
## *enabledValidationLayers*: checks if the application intends to use *validationLayers*
## *!checkValidationLayerSupport()*: checks if the validation layers are supported
## if both Condition are true it means that the validation layers was requested
## and the it is supported meaning they are available
## but if the validation layer was requested and the and not available then
## the std::runtime_error will output it's message

###### we can now run the Code
### finally we can now modify the VKInstanceCreateInfo struct instantiation to include the validation layer names if they are enabled
##  Code Example

                  if(enableValidationlayers){
                          createInfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
                          createInfo.ppEnabledLayerNames = validationlayers.data();
                  }else{
                          createInfo.enabledLayerCount = 0;
                    
                  }
## the if(enabledValidationlayers) is typically set based on whether the application is running on debug or development mode 
## createInfo.enabledLayerCount: is a field in the VKinstanceCreatInfo{} struct that specifies the number of validation layers
## static_cast<uint32_t>(validationlayers.size())
## validationlayers.size(): returns the number of elements in the validationlayers, which is a list of layers to enabled
## the std::vector.size(): method has a return type of size_t which is garanteed to be large enough t contain the size of the 
## largest object or dynamic array the host system can handle making it suitable to represent the size of collections i.e dynamic array
## so because validationlayers.size() has a size_t return type we have to typecast it to uint32_t type using static_cast and because
## uint32_t is portable in different systems
### Notice: that after we have typecasted the return type of validationlayers.size() from size_t --> uint32_t 
###         it means we only have the size of array meaning number of elements Now we have to get the Names of enabledLayers
## crateInfo.ppEnableLayerNames: this field in VKInsanceCreateInfo{} struct points to an array of c-style strings constaining the
## names of the validation layers to enable
### Validationlayers.data():we know that Validationlayers is an object name to the VK_LAYER_KHRONOS_validation that we typecasted
### to const char pointer and stores inside a const std::vector
##  data(): is a vector method. it returns a pointer to the Underlaying array of pointers. const char* managed by std::vector
##  which vulkan uses to access the layer names
## createInfo.enabledLayerCount = 0: if enablevalidationlayers is false the code sets createInfo.enabledLayerCount to 0
## indicating that no validation layers will be enabled
Note: createInfo.ppEnableLayerNames is not set to 0 in this case because vulkan will ignore it when createInfo.enableLayerCount is 0
### if the check was successfull then *vkCreateInstance()* should not ever return a VK_ERROR_LAYER_NOT_PRESENT error, now run the program



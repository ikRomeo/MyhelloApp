#ifndef IKPIPELINE_HPP
#define IKPIPELINE_HPP


#include "ikDeviceEngine.hpp" // we will include it to have access to ikDeviceEngine.cpp functions
#include <string>
#include <vector>

namespace ikE {


    /* we will now add pipelincConfigInfo struct that will contain the data that specifies how we want
       to configure the pipeline the reason we are putting it here instead of inside the iKePipeline class is so that our application
       layer code can easily be able to configure the pipeline completely and able to share the configuration between multiple
       pipelines
       inside the PipelineConfigInfo we need to define the different stages of how our pipeline works
       1. VkViewport struct defines how this transform occurs,both spatially (x position,y position,width and height) 
          and in depth which is the (z) position note to remember x is horizontal and y is vertical positions
          it defines how a portion of the framebuffer is used to rendering and it is part of the rendering stage
       2. VkRect2D struct defines a 2D rectangle in pixel space. it is used to specify regions in frame buffer
          i.e The scissors rectangle in(vkCmdsetScissor) , the render area in VkRenderPassBeginInfo and other situations
          where vulkan needs rectangular region
          The VkViewport maps the normalized device cordinate(NDC) to frambuffer space which is (scaling and depth)
          VkRect2D restrict where the framebuffer fragments are kept
       3. VkPipelineViewportStateCreateInfo struct defines how many viewports and scissors the pipeline will use
          and provides pointers and arrays of those viewports and scissors. it essentially tells vulkan how to 
          handle transforming normalized device cordinate(NDC) into framebuffer cordinates(viewports) and where
          to clip pixels(scissors) notice that it makes use of the VkViewport and VkRect2D structs
       4.VkPipelineInputAssemblyStateCreateInfo struct defines how vertices are interpreted as primitives and
         converts (points,lines,triangles) into fragments(potential pixels)
         it applies rules like polygon filling mode, culling, depth bias, line width e.t.c
         this stage is crucial for how the primitives appear on screen
      5. VkPipelineMultisampleStateCreateInfo struct configures multisampling behavior during rasterization stage
         of the vulkan graphics pipeline and multisampling is a technique to reduce aliasing(jagged edges) by
         sampling multiple points per pixel . it improves image quality, especially at edges of geometry
      6. VkColorBlendAttachmentState struct configures how color blending is performed per attachment in the
         vulkan graphics pipeline in the sense that after the fragment shader outputs a color for a pixel,
         The color blending stage determines how the color is combined with the existing color in the framebuffer
         it is essentioal for effects like transperancy, additive blending and more
      7. VkPipelineColorBlendStateCreateInfo struct describes the global color blending state of a graphics pipeline, 
         which governs how the colors output by the fragment shader are combined with the existing colors in the
         framebuffer attachment
      8. VkPipelineDepthStencilStateCreateInfo struct configures the depth and stencil testing stage of vulkan graphics
         pipeline. Depth and stencil testing happen after fragment shading but before the fragment color is written to 
         the framebuffer. they control per-pixel visibility and effects like masking


       */
    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator = (const PipelineConfigInfo&) = delete;
       //new
        VkPipelineViewportStateCreateInfo viewportInfo;
        //
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo  multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        //new
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        //
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class ikePipeline {
    public:
        /* we will include IkeDeviceEngine , PipelineConfigInfo struct to the ikePipeline constructor and in createGraphicsPipeline we will include the PipelineConfigInfo struct
          then create a destructor notice that the danglin braces has no semicolon
        
        */
        ikePipeline(IkeDeviceEngine& device,
                     const std::string& vertFilepath, 
                     const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
        ~ikePipeline();

        //we delete the copy constructors
        ikePipeline(const ikePipeline&) = delete;
        ikePipeline& operator=(const ikePipeline&) = delete;

        // needs explanation
        void bind(VkCommandBuffer commandBuffer);


       // static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    private:
        static std::vector<char> readFile(const std::string& filepath);

        /*GraphicsPipeline consist of multiple shader stages, multiple fixed function pipeline stages meaning we can not hard
          code this stage but only apply rules on how we want it to operate, and a pipeline layout*/
        void createGraphicsPipeline(const PipelineConfigInfo& configInfo,const std::string& vertFilepath, const std::string& fragFilepath);

        /* we will create a vector to our code and a pointer to the shaderModule  which is also a pointer making it shaderModule a pointer to a pointer
        
        */
        void createShaderModule(const std::vector<char>&  code, VkShaderModule* shaderModule);

        /* we will instance IkeDeviceEngine using reference which makes it an agregate because it will outlive the rest functions
           notice the remaining handles are typedef pointers
        */
        IkeDeviceEngine& IkeDevice;
        VkPipeline graphicsPipeline;
        // VkShaderModule is a handle
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };



}//namespace



#endif 

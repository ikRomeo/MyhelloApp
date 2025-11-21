#include "ikPipeline.hpp"
#include "ikEngineModel.hpp"
//std
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>


namespace ikE {

    /* notice how we start the constructor by using the aggregate relationship instance IkeDevice and using the reference of IkeDeviceEngine device as an argument*/
    ikePipeline::ikePipeline(IkeDeviceEngine& device, 
                             const std::string& vertFilepath, 
                             const std::string& fragFilepath, 
                             const PipelineConfigInfo& configInfo) : IkeDevice(device){
        createGraphicsPipeline(configInfo,vertFilepath, fragFilepath);
    }

    //destructor
    ikePipeline::~ikePipeline() {
        vkDestroyShaderModule(IkeDevice.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(IkeDevice.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(IkeDevice.device(), graphicsPipeline, nullptr);
    }

     std::vector<char> ikePipeline::readFile(const std::string& filepath) {

         // with the input file stream we can read from the file and its
         // argument are 3 . the file, ios::ate opens the file and move
         // the read pointer to the end so that we can get the size
         // we can know the size immediately using tellg
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        //throws error if the file can not be opened
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file" + filepath);
        }
        //we get the file size 
        // there will be no runtime checks because of static_cast
        // tellg returns the current position of the file pointer which
        //is because of set::ios::ate meaning at the end of file
        // so we know the file size thanks to tellg()
        size_t fileSize = static_cast<size_t>(file.tellg());

        // we allocate buffer for the file we want to read via a vector with type char and the file size
        std::vector<char> buffer(fileSize);

        // seekg() sets the position of the input pointer it can also jump to 
        // a specific position if we want but here we move back to 0 index
        // meaning it moves the file pointer back to the 
        // beginning so we can read it
        file.seekg(0);
        // we read the data and we also have the size
        // meaning we will read the file data until we get to the end of the file
        file.read(buffer.data(), fileSize);
        //we close the file and return the buffer
        file.close();
        return buffer;
    }

     // in crateGraphicsPipelin we have the PipelineConfigInfo struct that has
     // the whole pipe line states the vert and frag file paths
     //
     // we call the readFile() function to read both files
     //
    void ikePipeline::createGraphicsPipeline(const PipelineConfigInfo& configInfo,
                                             const std::string& vertFilepath,
                                             const std::string& fragFilepath) {
        
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "can not create graphics pipeline :: no pipelineLayout provided in configInfo");

        assert(configInfo.renderPass != VK_NULL_HANDLE &&
            "can not create graphics pipeline :: no renderPass provided in configInfo");
        //we read both frag and vert via readFile()
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);
        // ShaderStage
        //we instance VkPipelineShaderStageCreateInfo struct with an array of
        // two components via shaderStages which is the vert and the frag shader
        // both components with number 0 and 1 uses the same struct which is
        // the VkPipelineShaderStageCreateInfo that tells vulkan which shader
        // module to use
        // .sType alway set this to identify what structure type it is for
        //   vulkan to use before anything
        // .stage specifies the stage be it Vk_Shader_Stage_vertex_bit and
        // .module which is vertShaderModule is a compile SPIR-v shader module
        //   for vertex shader and fragment shader via createShaderModule()
        // .pName is the Entry function name in GLSL shader which is usually "main"
        // .flags = 0 it is reserved for fututue use and it is an enum
        //  that represents different shader stages or group of stages in vulkan
        //  it is used in vulkan structs to specify which shader stages to apply
        //  certain settings and create pipeline shader stages
        // .pNext No extension struture chained so it is set to nullptr by default
        // .pSpecialization because we don't want to customize the shader behavior
        //  for now by telling the compiler to generate specialized code paths
        //  we will simply assign nullptr to pSpecialization which is a pointer 
        //  variable and instance to VkSpecialization
        //  we will also do the same to the vert by adding the seven members
        //  we just described the only difference will be in the .sType,.stage.module
        //  where we use the respective fragShaderModule
        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        //we tell our pipeline to use the ikEngineModel here
        auto bindingDescriptions = ikEngineModel::Vertex::getBindingDescriptions();
        auto attributeDescriptions = ikEngineModel::Vertex::getAttributeDescriptions();

        //this tells us how we interprete the vertex buffer
        // meaning how data should be read from vertex buffer and
        // interpreted by the GPU inside the graphics pipeline
        // and it is a fixed function
        // we first declare and zero initialized the structure
        // .sType Specifies the type of structure we are using
        // .vertexAttributeDescriptionCount = 0 is a uint32_t data type that will be changed after we create the ikEngineModel to attributeDescriptions()
        // .VertexBindingDescriptionCount = 0 is a uint32_data type that will also be change after we create the ikEngineModel to bindingDescriptons()
        // .pvertexAttributeDescriptions = nullptr means no per-vertex that will also be changed after we create the ikEngineModel to attributeDescriptions.data()
        //  attributes are being passed because AttributeDescription
        //  and AttributeCounts are us to tell vulkan how to interprete
        //  vertex buffer in memory because it describes each vertex attributes
        //  e.g position,normal,color,texcoord it maps parts of vertex buffer
        //  into shaders input variable i.e layout(location = 0)
        // .pVertexBindingDescriptions = nullptr  because we have
        //  not yet created vkCmdBindVertexBuffers which needs to use
        //  this to vulkan which binding slots it refers to , how much to 
        //  advance in memory for each vertex , whether to step per vertex
        //  or per instance so we make nullptr for now Note that
        //  this and AttributeDescriptions with AtrributewCount and BindingDescriptwork together
        //  Notice the use of static_cast which converts a type to another i.e int to float or vice versa
        //  and it cannot cast pointer to another pointer that we do with reinterprete cast
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        

     


        //VkGraphicsPipelineCreateInfo contains all the fixed functions and programmable stage
        // configurations for a vulkan graphics pipeline because vulkan pipelines are immutable
        // we describe everything at once
        // .sType is the field that tells vulkan what type of struct we are creating and want to use
        // .stageCount = 2 because we are using two shader stage vert and frag shader
        // .pStages is assigned the instance of VkPipelineShaderCreateInfo shaderStages
        //  which is responsible for shader type, compile SPIR-V module and entry point which is main
        //  the pStages and stageCount are both shader stages
        // the fixed non programmable stage now begin with
        // .pVertexInputState controls how vertex data is read from the buffers which is
        //  (binding, attributes, strides, formats)
        // .pInputAssemblyState Controls how vertices are assembled into primitives
        //  i.e VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        // .pViewportState Defines viewports and scissor rectangle how NDC cordinates 
        //  are mapped to framebuffer space
        // .RasterizationState converts primitives into fragments
        //  Settings include polygon mode (fill, wireframe), culling, front face winding
        // .pMultisampleState is for Anti-aliasing control(sample count, sample shading)
        // .Controls how fragment shader output colors are blended with existing framebuffer
        //  colors
        // .pDepthStencilState controls depth testing, depth writes, stencil operations
        // .DynamicState if set allows certain state i.e (viewport,scissors, e.t.c)
        //  to be changed without recreating the pipeline the nullptr means no dynamic state
        // .layout defines descriptor sets and push constants basically how shaders recieve
        //  data from CPU/GPU memory
        // .renderPass defines framebuffer attachments i.e (color,depth,e.t.c)
        // .subpass index tells vulkan which subpass this pipeline will be used in
        //  The derivative pipelines are basePipelineIndex and basePipelineHandle
        //  is a way to allow pipelines that share much of their configuration with
        //  another pipeline for faster creation here it is unused
        // then we call vkCreateGraphicsPipelines with (the device,because no pipeline
        // cache is used we make it VK_NULL_HANDLE, 1 because we are creating exactly one pipeline,
        // our configuration struct which is &pipelineInfo, we are not using any memory allocation so it is nullptr,
        // then the output handle for the created pipeline which we reference with &graphicsPipeline)
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;


        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(IkeDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error(" failed to create graphics pipeline");
        }


    }


    //createShaderModule is responsible for creating the frag and vert shader modules
    // it has two arguments a dynamic array with a reference code and VkShaderModule which is a handle
    // that is responsible for where the shader modules will be stored
    void ikePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
        //we declare and zero initializes a VkShaderModuleCreateInfo struct
        // that will hold the information needed to create a vulkan shader module
       
        VkShaderModuleCreateInfo createInfo{};
        //.sType this is required by vulkan to identify the type of structure
        //.codeSize sets the field to the size in bytes of shader bytecode
        //  and it tells the vulkan how large the shader code is
        //.pCode expects a pointer to uint32_t because SPIR-v bytecode
        // is and array of 32-bit words
         // we need reinterpret_cast because of unrelated types or pointers
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        // we then call vkCreateShaderModule()
        if (vkCreateShaderModule(IkeDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }


    }
    //need explanation
    /*
      VK_PIPELINE_BIND_POINT_GRAPHICES specifies binding as a graphics pipeline
      
      */
    void ikePipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }


    /*in the PipelineConfigInfo the first stage  
      .sType is the structure vulkan indentifies with
      .topology is the first stage of our pipeline where it takes list of vertexes and group them into geometry
      .
      
      */

    void ikePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {

        //PipelineConfigInfo configInfo{};
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;


       

        /*this stage breaks up the geometry into fregments*/
        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;    //Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;             //Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;       //Optional

        /*multisample explains how the rasteriza handles the edges of geometry
          without multismapling enabled a fragment is considered completly in or out 
          of a triangle*/
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;                //Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;                  //Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;       //Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;            //OPtional

         /*color blending controls how we combin colors in the frame buffer*/
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     //Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;   //Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;               //Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;    //Optiional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  //Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              //Optional


        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;   //Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  //Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  //Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  //Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  //Optional

        /*this is an additional attachment to the frame buffer and stores a value for every pixel like 
          color attachment stores the color*/
        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;    //Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;    //Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {}; //Optional
        configInfo.depthStencilInfo.back = {};  //Optional

        configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;





        //return configInfo;

    }




}//namespace

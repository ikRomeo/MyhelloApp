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
        auto &bindingDescriptions = configInfo.bindingDescriptions;
        auto &attributeDescriptions = configInfo.attributeDescriptions;

       
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        

     


     
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

        configInfo.bindingDescriptions = ikEngineModel::Vertex::getBindingDescriptions();
        configInfo.attributeDescriptions = ikEngineModel::Vertex::getAttributeDescriptions();



        //return configInfo;

    }




}//namespace

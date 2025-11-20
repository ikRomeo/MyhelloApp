#pragma once
#ifndef IKENGINEMODEL_HPP
#define IKENGINEMODEL_HPP


#include "ikDeviceEngine.hpp"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <vector>


namespace ikE {
	/*the purpose of this class is to be able to take vertex data created by
	 read in a file on the CPU and then allocate the memory and copy the data
	 over to the device GPU so that it can be rendered efficiently */

	class ikEngineModel {

	public:
		/*the Vertex struct represents a single vertex. Currently, it only has a glm::vec2 position
		  meaning this is a 2D vertex
		  The two static functions describe how Vulkan should interprete this struct when it reads it
		   in the vertex shader 
		   VkVertexInputBindingDescription tells Vulkan how to read the vertex buffer memory.
		   Essentially, it defines the stride(which is size of each vertex), which buffer you are using
		   and the rate at which to step through vertices
		   VkVertexInputAttributeDescription tells Vulkan how to interprete each piece of data inside a vertex
		   (position,color,normal,uv, e.t.c) and maps it to a shader input location.
		   In simple terms it tells Vulkan how big each vertex is and how to move between them in the vertex buffer

		   and they a both used in the VkPipelineVertexInputStateCreateInfo struct which is in the ikPipelince.cpp file
		   you will Notice that they are both stored in dynamic array std::vector
		   
		   */
		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;  //new

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

		};


		/* the constructor
		   Takes the Device wrapper which is the class IkeDeviceEngine with the help of a reference(&)
		   operator and the vertex data which is the struct that has the (glm,binding and attribute as members
		   then we create the destuctor with the tilder and empty function*/
		ikEngineModel(IkeDeviceEngine &device, const ikEngineModel::Builder &builder);  
		~ikEngineModel();

		ikEngineModel(const ikEngineModel&) = delete;
		ikEngineModel& operator = (const ikEngineModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);


	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);


		IkeDeviceEngine &IkeDevice;
		
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

	};












}//namespace

#endif
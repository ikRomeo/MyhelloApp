#pragma once
#ifndef IKENGINEMODEL_HPP
#define IKENGINEMODEL_HPP


#include "ikDeviceEngine.hpp"
#include "ikbuffer.hpp"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include<memory>
#include <vector>


namespace ikE {
	/*the purpose of this class is to be able to take vertex data created by
	 read in a file on the CPU and then allocate the memory and copy the data
	 over to the device GPU so that it can be rendered efficiently */

	class ikEngineModel {

	public:
		
		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;  

			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);

		};


		/* the constructor
		   Takes the Device wrapper which is the class IkeDeviceEngine with the help of a reference(&)
		   operator and the vertex data which is the struct that has the (glm,binding and attribute as members
		   then we create the destuctor with the tilder and empty function*/
		ikEngineModel(IkeDeviceEngine &device, const ikEngineModel::Builder &builder);  
		~ikEngineModel();

		ikEngineModel(const ikEngineModel&) = delete;
		ikEngineModel& operator = (const ikEngineModel&) = delete;

		static std::unique_ptr<ikEngineModel> createModelFromFile(IkeDeviceEngine& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);


	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);


		IkeDeviceEngine &IkeDevice;
		
		std::unique_ptr<IkBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<IkBuffer> indexBuffer;
		uint32_t indexCount;

	};












}//namespace

#endif
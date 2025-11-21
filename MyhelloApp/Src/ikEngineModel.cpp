#include "ikEngineModel.hpp"
#include "ikUtils.hpp"
//libs
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
//std
#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std {
	template <>
	struct hash<ikE::ikEngineModel::Vertex> {
		size_t operator()(ikE::ikEngineModel::Vertex const& vertex) const {
			size_t seed = 0;
			ikE::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace ikE {

	/*in the constructor 
	  the constructor has 2 arguments and initlizer list an argument
	  Member initializer list (:IkeDevice(device))
	  This initializes the reference IkeDevice(the Vulkan device wrapper) directly before the constructor body runs
	  createVertexBuffer(vertices)
	  immediately calls the private function that uploads the vertex data to GPU memory By doing this in the constructor
	  we ensure that once an ikEngineModel object exists,the GPU buffer is ready 
	  meaning when we create a model, it automatically builds its vertex buffer and uploads data to the GPU
	  Notice that the initilizer has IkeDevice which is the reference(&) instance of IkeDeviceEngine placed in the
	  private section of the class ikEngineModel
	  inside the initialize list function we call the createVertexBuffers() with vertices as argument
	  */
	ikEngineModel::ikEngineModel(IkeDeviceEngine &device, const ikEngineModel::Builder& builder) : IkeDevice(device) {
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);

	}
	/* in the destructor
	   we call vkDestroyBuffer with 3 arguments (IkeDevice which is the initilizer .device() function and vertexBuffer of type VkBuffer and nullptr) this frees vkhandle
	   we call vkFreeMemory  with 3 arguments (the vertexBufferMemory of type VkDeviceMemory and nullptr) this frees the memory that was allocated for the buffer
	   */
	ikEngineModel::~ikEngineModel() {}

	std::unique_ptr<ikEngineModel> ikEngineModel::createModelFromFile(IkeDeviceEngine& device, const std::string& filepath) {

		Builder builder{};
		builder.loadModel(filepath);

		std::cout << "Vertex count: " << builder.vertices.size() << "\n";
		return std::make_unique<ikEngineModel>(device, builder);

	}



	/*
	 createVertexBuffers we access the struct which is in the hpp file
	 and put it in a dynamic array that is a vector so that we can access it's size
	 then reference (&) it with an instance vertices
	 
	 vertexCount which is of type uint32_t is assigned vertices which is of type struct and notice we use static_cast
	 to cast it to uint32_t because vulkan prefers fixed width types
	 assert which is member of the header cassert is used to check for valid number of vertices
	 in this case at least 3 for a triangle
	 VkDeviceSize is of uint64 and we use to get the total memory required by multiplying the
	 size of one vertex by how many we have
	 we then call IkeDevice to create the Buffer with  five arguments via createBuffer(it creates a VkBuffer given size and usage flag, 
	 it allocates VkDeviceMemory for that buffer and Binds the memory to the buffer ) we will also modify in the future
	 we then create a pointer to a void data type
	 we then call vkMapMemory() with 6 arguments (it maps GPU memory into the CPU's address space, after mapping, data points to a region of memory where we can write our vertex byte directly)
	 we then call memcpy(that copies raw bytes from our vertices vector to the mapped GPU memory, vertices.data() returns a pointer to the vertex in the CPU array)
	 we then call vkUnmapMemory() Unmaps the memory after copying telling vulkan that we are done, Since the memory is coherent, we do not need to flush it
     this 3 actions will now make the vertex data to live in GPU-accessible memory

	 */

	void ikEngineModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3!");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		IkBuffer stagingBuffer{
			IkeDevice,vertexSize,vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());
		
		vertexBuffer = std::make_unique<IkBuffer>(
			IkeDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	

		IkeDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

	

	}

	void ikEngineModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		
		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof((indices[0]));

		IkBuffer stagingBuffer{
			IkeDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());
		indexBuffer = std::make_unique<IkBuffer>(
			IkeDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		
		);
		IkeDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	
	}






	void ikEngineModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
	        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}


	/* Note that before drawing we must bind the vertex buffer
	   with the bind() that also takes the commandBuffer of type VkCommandBuffer 
	   buffers[] is the array of buffers to bind and in this case it is the VertexBuffer of type VkBuffer as its argument
	   offset[] is how far into each buffer to start reading in this case it is 0
	   then we call vkCmdBindingVertexBuffers() with 5 arguments
	   */
	void ikEngineModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	

	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	   }
	}

	/* here we have the vector argument then ikEngineModel the class and Vertex the nested struct and getBindingDescription a member of the static member of the struct Vertex
	   Notice that we did not use the static keyword here only in the hpp file */
	std::vector<VkVertexInputBindingDescription>ikEngineModel::Vertex::getBindingDescriptions() {


		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;

	}
	/*we now will update the attributeDescription to take 2 different parts*/
	std::vector<VkVertexInputAttributeDescription>ikEngineModel::Vertex::getAttributeDescriptions() {


		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back( {0,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)} );
		attributeDescriptions.push_back({ 1,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2,0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3,0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });


		return attributeDescriptions;


	}

	void ikEngineModel::Builder::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex ,uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};

				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
						
					};
				}
				//vertices.push_back(vertex);
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}


}//namespace
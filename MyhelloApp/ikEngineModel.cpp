#include "ikEngineModel.hpp"
//std
#include <vector>
#include <cassert>
#include <cstring>

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
	ikEngineModel::~ikEngineModel() {
		vkDestroyBuffer(IkeDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(IkeDevice.device(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(IkeDevice.device(), indexBuffer, nullptr);
			vkFreeMemory(IkeDevice.device(), indexBufferMemory, nullptr);
		}
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

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		IkeDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		
		void* data;
		vkMapMemory(IkeDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(IkeDevice.device(), stagingBufferMemory);

		IkeDevice.createBuffer(bufferSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		IkeDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(IkeDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(IkeDevice.device(), stagingBufferMemory, nullptr);

	}

	void ikEngineModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		
		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		IkeDevice.createBuffer(
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);


		void* data;
		vkMapMemory(IkeDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(IkeDevice.device(), stagingBufferMemory);

		IkeDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		IkeDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(IkeDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(IkeDevice.device(), stagingBufferMemory, nullptr);


	
	
	}






	/* with the draw() we then issue a draw command inside the commandBuffer which is of type VkCommandBuffer 
	   inside it's function we call vkCmdDraw that take 5 arguments
	   (the commandBuffer, 
	    vertexCount which is the number of vertices to draw, 
	    1 meaning instanceCount(for instancing -1 means a single draw,
	    0 means the firstVertex(start at vertex index 0,
	    0 means the firstInstance(state at instance index 0))
	*/
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
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	

	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;  // equivalent to the layout(location = 0) in vec2 position in shader.vert file
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);
	


		attributeDescriptions[1].binding = 0; // because we are both interleaving both colors to one binding it will still remain 0
		attributeDescriptions[1].location = 1;  // equivalent to the layout(location = 1) in vec3 position in shader.vert file
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // because it is vec3 we add B32 to the format
		attributeDescriptions[1].offset = offsetof(Vertex, color);  // here we use offsetof operator to calculate the vertex and color
		return attributeDescriptions;


	}



}//namespace
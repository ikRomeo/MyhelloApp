#ifndef IKWINDOW_HPP
#define IKWINDOW_HPP


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace ikE {
	class IkeWindow {
	public:
		IkeWindow(int w, int h, std::string name);
		~IkeWindow();

		IkeWindow(const IkeWindow&) = delete;
		IkeWindow& operator = (const IkeWindow) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		//Note explanation
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width),static_cast<uint32_t>(height) }; }
		
		//Note explanation
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;
		GLFWwindow* window;
	};
}

#endif
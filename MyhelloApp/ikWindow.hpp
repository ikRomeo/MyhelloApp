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
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized;  }


		//Note explanation
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:
		//Note explanation
		static void framebufferResizedCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		 int width;
		 int height;
		 bool framebufferResized = false;

		std::string windowName;
		GLFWwindow* window;
	};




}//namespace

#endif
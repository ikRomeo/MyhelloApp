#include "ikWindow.hpp"

#include <stdexcept>
namespace ikE {
	IkeWindow::IkeWindow(int w, int h, std::string name) : width(w), height(h), windowName{ name } {
		initWindow();
	}
	IkeWindow::~IkeWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void IkeWindow::initWindow(){
		glfwInit(); 
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizedCallback);
	}
	//Note explaination
	void IkeWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void IkeWindow::framebufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto ikewindow = reinterpret_cast<IkeWindow*> (glfwGetWindowUserPointer(window));
		ikewindow->framebufferResized = true;
		ikewindow->width = width;
		ikewindow->height = height;
	 }

}//namespace
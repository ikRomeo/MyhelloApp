




my project before the 

ikWindow.hpp file
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
	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;
		GLFWwindow* window;
	};
}

#endif

ikWindow.cpp file
#include "ikWindow.hpp"

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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}

}

First_App.hpp file

#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikWindow.hpp"
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run() {};

	private:
		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
	};

} //namepace
#endif //header guard



main.cpp file

#include "First_App.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    ikE::FirstApp app{};

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}



//then we build the project now lets fill up the run() function 
First_App.cpp

#include "First_App.hpp"

namespace ikE {
	void FirstApp::run() {
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();
		}
	}
}


//we go to ikWindow.hpp file initialize the shouldClose()

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

		bool shouldClose() { return glfwWindowShouldClose(window); }
	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;
		GLFWwindow* window;
	};
}

#endif

//go to First_App.hpp and remove the dangling braces in void run();

#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikWindow.hpp"
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run();

	private:
		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
	};

} //namepace
#endif //header guard

//go to the ikWindow.hpp and create function to delete the copy constructor and the copy operator

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
		
	private:
		void initWindow();

		const int width;
		const int height;

		std::string windowName;
		GLFWwindow* window;
	};
}

#endif













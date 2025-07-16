#include "First_App.hpp"

namespace ikE {
	void FirstApp::run() {
		while (!ikeWindow.shouldClose()) {
			glfwPollEvents();
		}
	}
}
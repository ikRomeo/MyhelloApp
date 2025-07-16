#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikWindow.hpp"
#include "ikPipeline.hpp"
#include "ikDeviceEngine.hpp"
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run();

	private:
		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
		IkeDeviceEngine ikeDeviceEngine{ ikeWindow };
        ikePipeline ikePipeline{"frag.spv","vert.spv"};
	};

} //namepace
#endif //header guard

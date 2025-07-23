#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP

#include "ikeWindow.hpp"
namespace Ike{
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		void run() {};

	private:
		IkeWindow ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
		};

} //namepace
#endif //header guard

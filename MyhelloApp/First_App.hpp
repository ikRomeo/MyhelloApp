#ifndef FIRST_APP_HPP
#define FIRST_APP_HPP


#include "ikDeviceEngine.hpp"
#include "ikgameObject.hpp"

#include "ikRenderer.hpp"
#include "ikWindow.hpp"
//std
#include <memory>
#include <vector>
namespace ikE {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGTH = 600;

		//because we have the constructors here we should also remember to delete the copy constructors 
		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator =(const FirstApp&) = delete;

		void run();

	private:
	
		void loadGameObjects();
	

		IkeWindow   ikeWindow{ WIDTH,HEIGTH,"HELLO GUYS" };
		IkeDeviceEngine ikeDeviceEngine{ ikeWindow };
		IkeRenderer IkRenderer{ ikeWindow,ikeDeviceEngine };

		std::vector<IkgameObject> gameObjects;
	};

} //namepace
#endif //header guard

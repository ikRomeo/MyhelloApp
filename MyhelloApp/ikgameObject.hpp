#pragma once
#ifndef IKEGAMEOBJECT_HPP
#define IKEGAMEOBJECT_HPP

#include "ikEngineModel.hpp"
//std
#include <memory>

namespace ikE{
	//this is used to move objects up and down
	struct Transform2dComponent {
		glm::vec2 translation{};  //position offset
		glm::vec2 scale{ 1.f,1.f };
		float rotation;


		glm::mat2 mat2() { 
			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);
			glm::mat2 rotMatrix{ {c, s}, {-s, c } };


			glm::mat2 scaleMat{ {scale.x, 0.f}, {.0f, scale.y} };
					return rotMatrix * scaleMat; }
	};

	class IkgameObject {
	public:
		using id_t = unsigned int;

		static IkgameObject createGameObject() {
			static id_t currentId = 0;
			return IkgameObject(currentId++);

		}
		IkgameObject(const IkgameObject&) = delete;
		IkgameObject& operator=(const IkgameObject&) = delete;
		IkgameObject(IkgameObject&&) = default;
		IkgameObject& operator=(IkgameObject&&) = default;


		//getId should be marked as const
		id_t getId() { return id; }
		std::shared_ptr<ikEngineModel> model{};
		glm::vec3 color{};
		Transform2dComponent transform2d{};

	private:
		IkgameObject(id_t objId) : id(objId){}
		id_t id;
	};



}//namespace
#endif
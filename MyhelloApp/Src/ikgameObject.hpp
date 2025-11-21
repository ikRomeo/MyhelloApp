#pragma once
#ifndef IKEGAMEOBJECT_HPP
#define IKEGAMEOBJECT_HPP

#include "ikEngineModel.hpp"
//libs
#include "glm/gtc/matrix_transform.hpp"
//std
#include <memory>

namespace ikE{
	//this is used to move objects up and down
	struct TransformComponent {
		glm::vec3 translation{};  //position offset
		glm::vec3 scale{ 1.f,1.f,1.f};
		glm::vec3 rotation{};

	//Matrix corresponds to translate * Ry * Rx * Rz * scale tranformatio
	//Rotation convention uses tait-bryan angles with axis order Y(1), x(2), Z(3)
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
		
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
		TransformComponent transform{};

	private:
		IkgameObject(id_t objId) : id(objId){}
		id_t id;
	};



}//namespace
#endif
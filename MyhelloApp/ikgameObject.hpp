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
		glm::mat4 mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f} };
		}


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
#pragma once
#ifndef IKFRAMEINFO_HPP
#define IKFRAMEINFO_HPP

#include "ikCamera.hpp"
#include "ikgameObject.hpp"

//lib
#include <vulkan/vulkan.h>


namespace ikE {

#define MAX_LIGHTS 10

	struct PointLight {
		glm::vec4 position{}; //ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};



	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		IkCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		IkgameObject::Map& gameObjects;
	};





}//namespace

#endif
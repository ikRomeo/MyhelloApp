#pragma once
#ifndef IKFRAMEINFO_HPP
#define IKFRAMEINFO_HPP

#include "ikCamera.hpp"

//lib
#include <vulkan/vulkan.h>


namespace ikE {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		IkCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};





}//namespace

#endif
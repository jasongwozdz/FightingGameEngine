#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "VulkanErrorCheck.h"

namespace VulkanWindow {
	
	void createSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR& surface) {

		VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
	
	}

}


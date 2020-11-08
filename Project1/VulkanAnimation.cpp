#include <iostream>
#include <GLFW/glfw3.h>
#include "Renderer.h"

int main()
{
	Renderer renderer;
	while (!glfwWindowShouldClose(renderer.getWindow())) {
		glfwPollEvents();
		renderer.drawFrame();
	}
	vkDeviceWaitIdle(renderer.getLogicalDevice());
}

#include <iostream>
#include <array>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "GameObjectManager.h"

#define NUM_OBJS 2

// Singletons
ResourceManager* resourceManager;
Renderer* renderer;
GameObjectManager gObjectManager;
BaseCamera* camera;
UserInterfaceImp* ui;
bool drawUi = false;
std::map<int, bool> keysHeld;

std::vector<int> gameObjectIds;

 void mouseCallback(GLFWwindow* window, double xpos, double ypos)
 {
	 if (!drawUi)
	 {
		 camera->updateMouse({ xpos, ypos });
	 }
 }

 void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
 {
	 if (key == GLFW_KEY_U)
	 {
		 if (keysHeld.find(GLFW_KEY_U) == keysHeld.end())
		 {
			 keysHeld[GLFW_KEY_U] = true;
		 }
		 else if (!keysHeld[GLFW_KEY_U])
		 {
			 drawUi = !drawUi;
			 renderer->setDrawUi(drawUi);
			 keysHeld[GLFW_KEY_U] = true;
		 }
		 else
		 {
			 keysHeld[GLFW_KEY_U] = false;
		 }
	 }
 }

void initScene() 
{
	std::string modelPath = "./Models/viking_room.obj";
	std::string texturePath = "./Textures/viking_room.png";
	Position pos = { 0, 0, 0 };
	gameObjectIds.push_back(gObjectManager.addGameObject(new GameObject(modelPath, texturePath, pos)));
	pos = { 0, 0, 1 };
	gameObjectIds.push_back(gObjectManager.addGameObject(new GameObject(modelPath, texturePath, pos)));
	camera = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, {0.0f, 0.0f, 1.0f});
	gObjectManager.addCamera(camera);
	gObjectManager.updateViewMatricies();
}

void initUi()
{
	ui = new UserInterfaceImp(renderer->getWindow());
	renderer->bindUI(ui);
	renderer->setDrawUi(drawUi);

}

void initSingletons() 
{
	resourceManager = new ResourceManager();
	renderer = new Renderer();
	renderer->setMousePosCallback(mouseCallback);
	renderer->setKeyboardCallback(keyboardCallback);
}

int main()
{
	initSingletons();
	initScene();
	Renderer::getSingleton().prepareScene();
	initUi();
	while (!glfwWindowShouldClose(renderer->getWindow())) {
		gObjectManager.updateViewMatricies();
		glfwPollEvents();
		if (ui != nullptr && drawUi)
			{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();	
			ImGui::ShowDemoWindow();
			bool show_demo_window = true;
			bool show_another_window = false;
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}
		}
	
		renderer->drawFrame();
	}
	vkDeviceWaitIdle(renderer->getLogicalDevice());
}

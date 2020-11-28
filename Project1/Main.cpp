#include <iostream>
#include <array>
#include <ctime>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "ResourceManager.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "DebugDrawManager.h"

// Singletons
ResourceManager* resourceManager;
Renderer* renderer;
GameObjectManager* gObjectManager;
DebugDrawManager* debugDrawManager;
BaseCamera* camera;
UserInterfaceImp* ui;
bool drawUi = false;
std::map<int, bool> keysHeld;

uint64_t endTime = 0;
uint64_t deltaTime = 0;

double lastMousePos[2] = { 0,0 };

std::vector<int> gameObjectIds;

std::map<int, int> keyBinds;

enum KEY_FUNCTIONS
{
	MOVE_FORWARD	= 0,
	MOVE_BACKWARD	= 1,
	STRAFE_LEFT		= 2,
	STRAFE_RIGHT	= 3,
	OPEN_UI			= 4,
	DRAW_GRID		= 5
};

 void mouseCallback(GLFWwindow* window, double xpos, double ypos)
 {
	 if (!drawUi)
	 {
		 camera->updateMouse({ xpos, ypos });
	 }
 }

 void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
 {
	 KEY_FUNCTIONS keyFunc;
	 switch (keyBinds[key])
	 {
	 case OPEN_UI:
		 keyFunc = OPEN_UI;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 drawUi = !drawUi;
			 renderer->setDrawUi(drawUi);
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 drawUi = !drawUi;
			 renderer->setDrawUi(drawUi);
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;

	 case MOVE_FORWARD:
		 keyFunc = MOVE_FORWARD;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;

	 case MOVE_BACKWARD:
		 keyFunc = MOVE_BACKWARD;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;

	 case STRAFE_LEFT:
		 keyFunc = STRAFE_LEFT;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;

	 case STRAFE_RIGHT:
		 keyFunc = STRAFE_RIGHT;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;
	 case DRAW_GRID:
		 keyFunc = STRAFE_RIGHT;
		 if (keysHeld.find(keyFunc) == keysHeld.end())
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;
	 }
 }

 void setUpKeybinds() {
	 keyBinds[GLFW_KEY_W]		= KEY_FUNCTIONS::MOVE_FORWARD;
	 keyBinds[GLFW_KEY_S]		= KEY_FUNCTIONS::MOVE_BACKWARD;
	 keyBinds[GLFW_KEY_A]		= KEY_FUNCTIONS::STRAFE_LEFT;
	 keyBinds[GLFW_KEY_D]		= KEY_FUNCTIONS::STRAFE_RIGHT;
	 keyBinds[GLFW_KEY_ESCAPE]	= KEY_FUNCTIONS::OPEN_UI;
	 keyBinds[GLFW_KEY_G]		= KEY_FUNCTIONS::DRAW_GRID;
 }

void initScene() 
{
	std::string modelPath = "./Models/viking_room.obj";
	std::string texturePath = "./Textures/viking_room.png";
	glm::vec3 pos = { 0, 0, 0 };
	gameObjectIds.push_back(gObjectManager->addGameObject(new GameObject(modelPath, texturePath, pos)));
	camera = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, {0.0f, 0.0f, 1.0f});
	debugDrawManager->drawGrid( { 255, 255, 255 }, 1 );
	gObjectManager->addCamera(camera);
	gObjectManager->updateViewMatricies();
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
	gObjectManager = new GameObjectManager();
	debugDrawManager = new DebugDrawManager();
	renderer->setMousePosCallback(mouseCallback);
	renderer->setKeyboardCallback(keyboardCallback);
}

void handleCameraMovement(uint64_t time)
{
	if (keysHeld[MOVE_FORWARD])
	{
		camera->moveForward(time);
	}
	else if (keysHeld[MOVE_BACKWARD])
	{
		camera->moveBackward(time);
	}
	else if (keysHeld[STRAFE_LEFT])
	{
		camera->strafeLeft(time);
	}
	else if (keysHeld[STRAFE_RIGHT])
	{
		camera->strafeRight(time);
	}
}

int main()
{
	initSingletons();
	initScene();
	Renderer::getSingleton().prepareScene();
	initUi();
	setUpKeybinds();
	while (!glfwWindowShouldClose(renderer->getWindow())) {
		uint64_t start = endTime;
		gObjectManager->updateViewMatricies();
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
		else
		{
			handleCameraMovement(deltaTime);
		}
		renderer->drawFrame();
		endTime = std::clock();
		deltaTime = endTime - start;
	}
	vkDeviceWaitIdle(renderer->getLogicalDevice());
}

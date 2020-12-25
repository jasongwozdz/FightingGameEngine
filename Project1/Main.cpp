#include <iostream>
#include <array>
#include <ctime>
#include <GLFW/glfw3.h>
#include <functional>
#include <crtdbg.h>
#include "Engine/Renderer/Renderer.h"
#include "Engine/ResourceManager.h"
#include "Engine/GameObject.h"
#include "Engine/GameObjectManager.h"
#include "Engine/DebugDrawManager.h"
#include "Engine/EngineSettings.h"
#include "Engine/Window.h"

#define MEMORY_LEAK_STATS_START _CrtMemState sOld; _CrtMemState sNew; _CrtMemState sDiff; _CrtMemCheckpoint(&sOld); //take a snapshot
#define MEMORY_LEAK_STATS_END \
	if (_CrtMemDifference(&sDiff, &sOld, &sNew)) \
    { \
        std::cerr<<"-----------_CrtMemDumpStatistics ---------/n";\
        _CrtMemDumpStatistics(&sDiff);\
        std::cerr<<"-----------_CrtMemDumpAllObjectsSince ---------/n";\
        _CrtMemDumpAllObjectsSince(&sOld);\
        std::cerr<<"-----------_CrtDumpMemoryLeaks ---------/n";\
        _CrtDumpMemoryLeaks();\
    }


// Singletons
ResourceManager* resourceManager;
Renderer* renderer;
GameObjectManager* gObjectManager;
DebugDrawManager* debugDrawManager;
PipelineManager* pipelineManager;
EngineSettings* engineSettings;

Window* window;

BaseCamera* camera;
UserInterface* ui;
bool drawUi = false;
std::map<int, bool> keysHeld;

float endTime = 0;
float deltaTime = 0;

double lastMousePos[2] = { 0,0 };

std::vector<int> gameObjectIds;

std::map<int, int> keyBinds;

AnimatedGameObject* goblin;

enum KEY_FUNCTIONS
{
	MOVE_FORWARD	= 1,
	MOVE_BACKWARD	= 2,
	STRAFE_LEFT		= 3,
	STRAFE_RIGHT	= 4,
	OPEN_UI			= 5,
	DRAW_GRID		= 6,
	STOP_ANIMATION  = 7
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
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
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 keysHeld[keyFunc] = false;
		 }
		 break;
	 case STOP_ANIMATION:
		 keyFunc = STOP_ANIMATION;
		 if (glfwGetKey(window, key) == GLFW_PRESS  && !keysHeld[keyFunc])
		 {
			 keysHeld[keyFunc] = true;
			 goblin->setAnimation(0);
		 }
		 else if (glfwGetKey(window, key) == GLFW_RELEASE)
		 {
			 goblin->setAnimation(-1);
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
	keyBinds[GLFW_KEY_K]		= KEY_FUNCTIONS::STOP_ANIMATION;
	keysHeld[MOVE_FORWARD]		= false;
	keysHeld[MOVE_BACKWARD]		= false;
	keysHeld[STRAFE_LEFT]		= false;
	keysHeld[STRAFE_RIGHT]		= false;
	keysHeld[OPEN_UI]			= false;
	keysHeld[DRAW_GRID]			= false;
	keysHeld[STOP_ANIMATION]	= false;
 }

void initScene() 
{
	//std::string animationModelPath("./Models/goblin.dae");
	//int goblinId = gObjectManager->addAnimatedGameObject(animationModelPath, "./Textures/viking_room.png", {0,0,0}, ANIMATION_PIPELINE);

	//AnimatedGameObject* goblin  = dynamic_cast<AnimatedGameObject*>(gObjectManager->getGameObjectPtrById(goblinId));
	//goblin->setAnimation(-1);
	//gObjectManager->getGameObjectPtrById(goblinId)->setScale({0.005, 0.005, 0.005});

	std::string modelPath = "./Models/viking_room.obj";
	std::string texturePath = "./Textures/viking_room.png";
	glm::vec3 pos = { 0, 0, 0 };
	int houseId = (gObjectManager->addGameObject(modelPath, texturePath, pos,BASIC_PIPELINE));
	

	camera = new BaseCamera({ 1.0f, 3.0f, 1.0f }, { -1.0f, -3.0f, -1.0f }, {0.0f, 0.0f, 1.0f});
	debugDrawManager->drawGrid( { 255, 255, 255 }, 1 );
	//debugDrawManager->addPoint({ 0,0,0 }, { 255, 0, 0 }, 1, true);
	gObjectManager->addCamera(camera);
	gObjectManager->updateViewMatricies();
}

void initUi()
{
	ui = new UserInterface(window->getGLFWWindow());
	renderer->bindUI(ui);
	renderer->setDrawUi(drawUi);
}

void onEvent(Events::Event& e)
{
	std::cout << e.getEventType() << std::endl;
}

void initSingletons() 
{
	gObjectManager = new GameObjectManager();
	resourceManager = new ResourceManager();
	engineSettings = new EngineSettings();
	engineSettings->init();
	pipelineManager = new PipelineManager();
	window = new Window();
	renderer = new Renderer();
	debugDrawManager = new DebugDrawManager();

	window->setEventCallback(onEvent);
	pipelineManager->init();
	renderer->init(window);
	debugDrawManager->init();
}

void handleCameraMovement(float time)
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

void cleanup()
{
	delete gObjectManager;
	delete engineSettings;
	delete pipelineManager;
	delete window;
	delete ui;
	delete renderer;
	delete debugDrawManager;
	delete camera;
	delete resourceManager;
}

//int main()
//{
//	initSingletons();
//	initScene();
//	//Renderer::getSingleton().prepareScene();
//	initUi();
//	setUpKeybinds();
//	while (!glfwWindowShouldClose(window->getGLFWWindow())) {
//		float start = endTime;
//		window->onUpdate();
//		gObjectManager->update(deltaTime);
//		if (ui != nullptr && drawUi)
//		{
//			ImGui_ImplVulkan_NewFrame();
//			ImGui_ImplGlfw_NewFrame();
//			ImGui::NewFrame();	
//			ImGui::ShowDemoWindow();
//			bool show_demo_window = true;
//			bool show_another_window = false;
//			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//			{
//				static float f = 0.0f;
//				static int counter = 0;
//
//				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
//
//				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
//				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
//				ImGui::Checkbox("Another Window", &show_another_window);
//
//				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
//				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
//
//				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
//					counter++;
//				ImGui::SameLine();
//				ImGui::Text("counter = %d", counter);
//
//				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//				ImGui::End();
//			}
//		}
//		else
//		{
//			handleCameraMovement(deltaTime);
//		}
//		renderer->drawFrame();
//		endTime = std::clock();
//		deltaTime = endTime - start;
//	}
//	
//	vkDeviceWaitIdle(renderer->getLogicalDevice());
//
//	cleanup();
//	_CrtDumpMemoryLeaks();
//	return 0;
//}

//#include "Application.h"
//
//int main(int argc, char** argv)
//{
//}

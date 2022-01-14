#include "CharacterSelectAppState.h"
#include "../InputHandler.h"
#include "Renderer/UIInterface.h"
#include "ResourceManager.h"
#include "Scene/Scene.h"
#include "EngineSettings.h"
#include "../FighterFileImporter.h"
#include "BaseCamera.h"
#include "DebugDrawManager.h"
#include "FightingAppState.h"
#include "Scene/Behaviors/ControllableCameraBehavior.h"
#include "Scene/Components/Camera.h"

#include "Input.h"

CharacterSelectAppState::CharacterSelectAppState(std::vector<std::string> fighterFiles, InputHandler* leftSideInputs, InputHandler* rightSideInputs, DebugDrawManager* debugDrawManager, GameBase* gameBase) :
	fighterFiles_(fighterFiles),
	scene_(Scene::getSingletonPtr()),
	resourceManager_(ResourceManager::getSingletonPtr()),
	ui_(UI::UIInterface::getSingletonPtr()),
	debugManager_(debugDrawManager),
	input_(Input::getSingletonPtr())
{
	//scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/SmallerSkybox");
	scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/skybox");
	inputs_[LEFT_SIDE] = leftSideInputs;
	inputs_[RIGHT_SIDE] = rightSideInputs;
	selectedFighters_[LEFT_SIDE] = fighterFiles_[0];
	selectedFighters_[RIGHT_SIDE] = fighterFiles_[0];
	memset(cursorPos_, 0, sizeof(int) * NUM_FIGHTERS);
	memset(prevCursorPos_, 0, sizeof(int) * NUM_FIGHTERS);
	memset(fighterState_, 0, sizeof(SelectionState) * NUM_FIGHTERS);
	initalizeCamera(gameBase);
	setupInput();
}

void CharacterSelectAppState::enterState()
{
	//load all fighters
	for (int i = 0; i < NUM_FIGHTERS; i++)
	{
		for (const std::string& file : fighterFiles_)
		{
			Entity* fighter = nullptr;
			fighter = loadFighter(file);
			if (fighter)
			{
				std::cout << "loaded fighter" << std::endl;
				fighters_[i].push_back(fighter);
			}
			else
			{
				std::cout << "ERROR: loading fighter from file - " << file << std::endl;
			}
		}
		for (Entity* fighter : fighters_[i])
		{
			fighter->getComponent<Renderable>().render_ = false;
			if (i == LEFT_SIDE)
			{
				flipFighter(fighter);
			}
		}
	}
}


AppState* CharacterSelectAppState::update(float deltaTime)
{
	//static bool skyboxSet = false;
	//if(!skyboxSet && Input::getSingleton().getButton("Fire2") > 0.0f)
	//{
	//	scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/skybox");
	//	skyboxSet = true;
	//	Entity* box = scene_->addEntity("Box");
	//	box->addComponent<Transform>(1.0f, 1.0f, 1.0f);
	//	ModelReturnVals modelVals = ResourceManager::getSingleton().loadObjFile("C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Models\\cube.obj");
	//	box->addComponent<Renderable>(modelVals.vertices, modelVals.indices, true ,"" , true);

	//	box = scene_->addEntity("Box");
	//	box->addComponent<Transform>(1.0f, 1.0f, 1.0f);
	//	box->addComponent<Renderable>(modelVals.vertices, modelVals.indices, true ,"" , true);

	//}

	//ui_->showImGuiDemoWindow();
	for (int i = 0; i < NUM_FIGHTERS; i++)
	{
		updateFighter(i, deltaTime);
	}
	drawFighterSelectGrid();
	if (bothFighterSelected_)
	{
		scene_->clearScene();
		return new FightingAppState(fighterFiles_[cursorPos_[LEFT_SIDE]], fighterFiles_[cursorPos_[RIGHT_SIDE]], debugManager_, inputs_[LEFT_SIDE], inputs_[RIGHT_SIDE]);
	}
	
	return nullptr;
}

std::vector<Entity*> CharacterSelectAppState::getCurrentlySelectedFighters()
{
	std::vector<Entity*> selctedFighters;
	selctedFighters.push_back(fighters_[0][cursorPos_[0]]);
	selctedFighters.push_back(fighters_[1][cursorPos_[1]]);
	return selctedFighters;
}

void CharacterSelectAppState::drawFighterSelectGrid()
{
	int currPos = 1;
	float width = EngineSettings::getSingletonPtr()->windowWidth;
	float height = EngineSettings::getSingletonPtr()->windowHeight;
	ui_->centerNextWindow();
	ui_->beginWindow("A", width, height, {width/2, height/2}, NULL, true);
	std::vector<std::pair<std::string, glm::vec4>> gridVals;
	for (int i = 0; i < fighterFiles_.size();i++)
	{
		glm::vec4 color = { 0,0,0,1 };
		if (i == cursorPos_[LEFT_SIDE])
		{
			color[0] += 1;
		}
		if (i == cursorPos_[RIGHT_SIDE])
		{
			color[1] += 1;
		}
		gridVals.push_back({ fighterFiles_[i], color });
	}
	const int GRID_WIDTH = 50;
	const int GRID_HEIGHT = 50;
	ui_->drawGrid(gridVals, {false, false, true}, 2, MAX_GRID_COLS, GRID_WIDTH, GRID_HEIGHT);
	ui_->EndWindow();
}

Entity* CharacterSelectAppState::loadFighter(const std::string& fighterFile)
{
	FighterFileImporter fighterFileImporter(fighterFile);
	AnimationReturnVals ret = resourceManager_->loadAnimationFile(fighterFileImporter.exportData_.modelFilePath);
	TextureReturnVals texVals = resourceManager_->loadTextureFile((std::string&)fighterFileImporter.exportData_.textureFilePath);
	Entity* entity = scene_->addEntity("Fighter");
	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Fighter");
	Renderable& mesh = entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter");
	mesh.render_ = false;
	Transform& transform = entity->addComponent<Transform>( -50, 0, 0 );
	//transform.rotation_ = glm::rotate(glm::mat4(1.0f), fighterFileImporter.exportData_.upRotation, { 1.0f, 0.0f, 0.0f });
	//transform.rotation_ = glm::rotate(transform.rotation_, fighterFileImporter.exportData_.rightSideRotation, { 0.0f, 1.0f, 0.0f });
	transform.setScale(0.019f);
	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");
	return entity;
}

void CharacterSelectAppState::updateCursor(int fighterIndex)
{
	static int lastXAxisInput[NUM_FIGHTERS] = { 0, 0 };
	static int lastYAxisInput[NUM_FIGHTERS] = { 0, 0 };
	int currentXAxisInput = -input_->getAxis(HorizontalInputs[fighterIndex]);//axis is flipped
	int currentYAxisInput = input_->getAxis(VerticalInputs[fighterIndex]);
	prevCursorPos_[fighterIndex] = cursorPos_[fighterIndex];
	if (currentYAxisInput < 0 && currentYAxisInput != lastYAxisInput[fighterIndex])//down pressed : move cursor down
	{
		if ((cursorPos_[fighterIndex] + MAX_GRID_COLS) < fighters_[fighterIndex].size())
		{
			cursorPos_[fighterIndex] += MAX_GRID_COLS;
		}

	}
	else if (currentYAxisInput > 0 && currentYAxisInput != lastYAxisInput[fighterIndex])//up pressed : move cursor up
	{
		if ((cursorPos_[fighterIndex] - MAX_GRID_COLS) >= 0)
		{
			cursorPos_[fighterIndex] -= MAX_GRID_COLS;
		}
	}
	else if (currentXAxisInput > 0 && currentXAxisInput != lastXAxisInput[fighterIndex])//right pressed : move cursor right 
	{
		if ((cursorPos_[fighterIndex] + 1) < fighters_[fighterIndex].size())
		{
			cursorPos_[fighterIndex] += 1;
		}

	}
	else if (currentXAxisInput < 0 && currentXAxisInput != lastXAxisInput[fighterIndex])//left pressed : move cursor left
	{
		if ((cursorPos_[fighterIndex] - 1) >= 0)
		{
			cursorPos_[fighterIndex] -= 1;
		}
	}
	lastYAxisInput[fighterIndex] = currentYAxisInput;
	lastXAxisInput[fighterIndex] = currentXAxisInput;
}

void CharacterSelectAppState::updateFighter(int fighterIndex, float deltaTime)
{
	switch (fighterState_[fighterIndex])
	{
		case(SelectionState::entering):
		{
			fighterEntering(fighterIndex, deltaTime);
			break;
		}
		case(SelectionState::idle):
		{
			fighterIdle(fighterIndex, deltaTime);
			break;
		}
		case(SelectionState::selected):
		{
			fighterSelected(fighterIndex, deltaTime);
			break;
		}
	}
}

void CharacterSelectAppState::fighterEntering(int fighterIndex, float deltaTime)
{
	updateCursor(fighterIndex);
	checkIfFighterCursorMoved(fighterIndex);
	if (checkFighterSelected(fighterIndex)) return;
	EnteringStateData& stateData = enteringStateData_[fighterIndex];
	glm::vec3 pos1;
	glm::vec3 pos2;
	float normalizedTime;
	if (fighterIndex == LEFT_SIDE)
	{
		pos1 = stateData.leftFighterPickedTranslation[0].first;
		pos2 = stateData.leftFighterPickedTranslation[1].first;
		normalizedTime = stateData.currTime / stateData.leftFighterPickedTranslation[1].second;
	}
	else
	{
		pos1 = stateData.rightFighterPickedTranslation[0].first;
		pos2 = stateData.rightFighterPickedTranslation[1].first;
		normalizedTime = stateData.currTime / stateData.rightFighterPickedTranslation[1].second;
	}
	if (normalizedTime > 1)
	{
		stateData.currTime = 0;
		fighterState_[fighterIndex] = SelectionState::idle;
		return;
	}
	glm::vec3 newPos = (pos2 * normalizedTime) + (pos1 * (1.0f - normalizedTime));
	Entity* selectedFighter = fighters_[fighterIndex][cursorPos_[fighterIndex]];
	Transform& transform = selectedFighter->getComponent<Transform>();
	transform.position_ = newPos;
	stateData.currTime += deltaTime;
}

void CharacterSelectAppState::fighterIdle(int fighterIndex, float deltaTime)
{
	updateCursor(fighterIndex);
	checkIfFighterCursorMoved(fighterIndex);
	checkFighterSelected(fighterIndex);
}

void CharacterSelectAppState::fighterSelected(int fighterIndex, float deltaTime)
{
	for (int i = 0; i < NUM_FIGHTERS; i++)
	{
		if (fighterState_[i] != SelectionState::selected)
			return;
	}
	fighters_[fighterIndex][cursorPos_[fighterIndex]]->getComponent<Renderable>().render_ = false;
	bothFighterSelected_ = true;
}

void CharacterSelectAppState::flipFighter(Entity* entity)
{
	Transform& transform = entity->getComponent<Transform>();
	transform.rotateAround(180.0f, Transform::worldUp);
}

void CharacterSelectAppState::transitionState(int fighterIndex, SelectionState state)
{
	fighterState_[fighterIndex] = state;
	switch(state)
	{
		case SelectionState::entering:
		{
			enteringStateData_[fighterIndex].currTime = 0;
			break;
		}
		case SelectionState::idle:
		{
			setFinalFighterPos(fighterIndex);
			break;
		}
		case SelectionState::selected:
		{
			setFinalFighterPos(fighterIndex);
			selectedStateData_[fighterIndex].currentFrame = 0;
			setAnimation(fighterIndex, "Attack");
			break;
		}
	}	
}

void CharacterSelectAppState::checkIfFighterCursorMoved(int fighterIndex)
{
	if (prevCursorPos_[fighterIndex] != cursorPos_[fighterIndex])
	{
		transitionState(fighterIndex, SelectionState::entering);
		fighters_[fighterIndex][prevCursorPos_[fighterIndex]]->getComponent<Renderable>().render_ = false;
		fighters_[fighterIndex][cursorPos_[fighterIndex]]->getComponent<Renderable>().render_ = true;
		setAnimation(fighterIndex, "Idle");
		return;
	}
}

bool CharacterSelectAppState::checkFighterSelected(int fighterIndex)
{
	if ((inputs_[fighterIndex]->currentAttackInput_ & FightingGameInput::Action::light) > 0)
	{
		transitionState(fighterIndex, SelectionState::selected);
		return true;
	}
	return false;
}

void CharacterSelectAppState::setAnimation(int fighterIndex, std::string animation)
{
	fighters_[fighterIndex][cursorPos_[fighterIndex]]->getComponent<Animator>().setAnimation(animation);
}


void CharacterSelectAppState::setFinalFighterPos(int fighterIndex)
{
	Entity* selectedFighter = fighters_[fighterIndex][cursorPos_[fighterIndex]];
	Transform& transform = selectedFighter->getComponent<Transform>();
	if (fighterIndex == LEFT_SIDE)
	{
		transform.position_ = enteringStateData_->leftFighterPickedTranslation[enteringStateData_->leftFighterPickedTranslation.size() - 1].first;
	}
	else
	{
		transform.position_ = enteringStateData_->rightFighterPickedTranslation[enteringStateData_->leftFighterPickedTranslation.size() - 1].first;
	}
}

void CharacterSelectAppState::setupInput()
{
	input_->addAxis("Horizontal_P2", GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
	input_->addAxis("Vertical_P2", GLFW_KEY_UP, GLFW_KEY_DOWN);
}

void CharacterSelectAppState::initalizeCamera(GameBase* gameBase)
{
	camera_ = scene_->addEntity("CharacterSelectCamera");
	camera_->addComponent<Transform>(0, 0, 0);
	camera_->addComponent<Camera>(camera_);
	camera_->addComponent<Behavior>(new CharacterSelectCameraBehavior(camera_, this));
	scene_->setActiveCamera(camera_);
}

void onEvent(Events::Event& keyEvent)
{
	Events::EventDispatcher dispatcher(keyEvent);

}

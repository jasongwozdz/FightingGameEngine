#include "CharacterSelectAppState.h"
#include "../InputHandler.h"
#include "NewRenderer/UIInterface.h"
#include "ResourceManager.h"
#include "Scene/Scene.h"
#include "EngineSettings.h"
#include "../FighterFileImporter.h"
#include "BaseCamera.h"
#include "DebugDrawManager.h"
#include "FightingAppState.h"


CharacterSelectAppState::CharacterSelectAppState(std::vector<std::string> fighterFiles, InputHandler* leftSideInputs, InputHandler* rightSideInputs, DebugDrawManager* debugDrawManager) :
	fighterFiles_(fighterFiles),
	scene_(Scene::getSingletonPtr()),
	resourceManager_(ResourceManager::getSingletonPtr()),
	ui_(UI::UIInterface::getSingletonPtr()),
	debugManager_(debugDrawManager)
{
	//scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/skybox");
	inputs_[LEFT_SIDE] = leftSideInputs;
	inputs_[RIGHT_SIDE] = rightSideInputs;
	selectedFighters_[LEFT_SIDE] = fighterFiles_[0];
	selectedFighters_[RIGHT_SIDE] = fighterFiles_[0];
	memset(cursorPos_, 0, sizeof(int) * NUM_FIGHTERS);
	memset(prevCursorPos_, 0, sizeof(int) * NUM_FIGHTERS);
	memset(fighterState_, 0, sizeof(SelectionState) * NUM_FIGHTERS);
	camera_ = new BaseCamera({ -10, 0, 0, }, { 1, 0, 0 }, { 0, 0, 1 });
	cameraIndex_ = scene_->addCamera(camera_);
	scene_->setCamera(cameraIndex_);
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
			if (i == RIGHT_SIDE)
			{
				flipFighter(fighter);
			}
		}
	}
}

AppState* CharacterSelectAppState::update(float deltaTime)
{
	//ui_->showImGuiDemoWindow();
	for (int i = 0; i < NUM_FIGHTERS; i++)
	{
		updateFighter(i, deltaTime);
	}
	drawFighterSelectGrid();
	if (bothFighterSelected_)
	{
		return new FightingAppState(fighterFiles_[cursorPos_[LEFT_SIDE]], fighterFiles_[cursorPos_[RIGHT_SIDE]], debugManager_, inputs_[LEFT_SIDE], inputs_[RIGHT_SIDE]);
	}
	return nullptr;
}

void CharacterSelectAppState::drawFighterSelectGrid()
{
	int currPos = 1;
	float width = EngineSettings::getSingletonPtr()->windowWidth;
	float height = EngineSettings::getSingletonPtr()->windowHeight;
	ui_->centerNextWindow();
	ui_->beginWindow("A", width, height, NULL, true);
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
	transform.rot_ = glm::rotate(glm::mat4(1.0f), fighterFileImporter.exportData_.upRotation, { 1.0f, 0.0f, 0.0f });
	transform.rot_ = glm::rotate(transform.rot_, fighterFileImporter.exportData_.rightSideRotation, { 0.0f, 1.0f, 0.0f });
	transform.setScale(0.019f);
	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation("Idle");
	return entity;
}

void CharacterSelectAppState::updateCursor(int fighterIndex)
{
	static int lastXAxisInput[NUM_FIGHTERS] = { 0, 0 };
	static int lastYAxisInput[NUM_FIGHTERS] = { 0, 0 };
	int currentXAxisInput = inputs_[fighterIndex]->currentMovementInput_[0];
	int currentYAxisInput = inputs_[fighterIndex]->currentMovementInput_[1];
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
	transform.pos_ = newPos;
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
	entity->getComponent<Transform>().scale_.x *= -1;
	entity->getComponent<Transform>().scale_.y *= -1;
	entity->getComponent<Transform>().scale_.z *= -1;

	glm::quat rot = entity->getComponent<Transform>().rot_;
	glm::quat flipRot(0.0f, 0.0f, 1.0f, 0.0f);
	entity->getComponent<Transform>().rot_ = flipRot * rot;
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
	if ((inputs_[fighterIndex]->currentAttackInput_ & Input::Action::light) > 0)
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
		transform.pos_ = enteringStateData_->leftFighterPickedTranslation[enteringStateData_->leftFighterPickedTranslation.size() - 1].first;
	}
	else
	{
		transform.pos_ = enteringStateData_->rightFighterPickedTranslation[enteringStateData_->leftFighterPickedTranslation.size() - 1].first;
	}
}

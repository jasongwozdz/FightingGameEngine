#include <iostream>
#include <filesystem>
#include <cstdio>
#include <math.h>
#include "FighterCreatorTool.h"
#include "../EntryPoint.h"
#include "NewRenderer/UIInterface.h"

FighterCreatorTool::FighterCreatorTool()
{
}

FighterCreatorTool::~FighterCreatorTool()
{
}

void FighterCreatorTool::onStartup()
{
	setCursor(true);
	ui_ = UI::UIInterface::getSingletonPtr();
	baseCamera_ = new BaseCamera({ 0.0, cameraDistanceFromFighter_, 0.0 }, { 0.0, -10.0, 0.0 }, { 0.0f, 0.0f, 1.0f });
	scene_->addCamera(baseCamera_);
	std::string error;
	//Remove when finished testing
	if (!LoadMesh("C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Models\\goblin.dae", error))
	{
		std::cout << error << std::endl;
	}

	addEventCallback(std::bind(&FighterCreatorTool::onEvent, this, std::placeholders::_1));
}

void FighterCreatorTool::drawHitboxes()
{
	std::vector<Hitbox>& currentHitboxes = CurrentAnimationData_.getCurrentHitboxData();
	for (int i = 0; i < currentHitboxes.size(); i++)
	{
		glm::vec3 angleOfRotation = { 0,0,0 };
		glm::vec3 color;
		if (currentHitboxes[i].layer_ == Hitbox::Push)
		{
			color = { 0, 255, 0 };
		}
		else if (currentHitboxes[i].layer_ == Hitbox::Hit)
		{
			color = { 255, 255, 0 };
		}
		else
		{
			color = { 255, 0, 0 };
		}

		if (currentObjectSelectedId_ == currentHitboxes[i].uniqueId_)
		{
			color = { 255, 255, 255 };
		}
			
		debugManager_->drawFilledRect(currentHitboxes[i].pos_, color, angleOfRotation, 0, -currentHitboxes[i].width_ / 2, currentHitboxes[i].width_ / 2, -currentHitboxes[i].height_ / 2, currentHitboxes[i].height_ / 2, currentHitboxes[i].uniqueId_);
	}
}

Hitbox* FighterCreatorTool::getHitboxById(const unsigned int id)
{
	std::vector<Hitbox>& currentHitboxes = CurrentAnimationData_.getCurrentHitboxData();
	for(Hitbox& hitbox : currentHitboxes)
	{
		if (hitbox.uniqueId_ == id)
		{
			return &hitbox;
		}
	}
	return nullptr;
}

int FighterCreatorTool::createHitboxId()
{
	static int hitboxId = 0;
	return ++hitboxId;
}

void FighterCreatorTool::onUpdate(float deltaTime)
{
	DrawMenuBar();
	debugManager_->addLine({ 0,0,0 }, { 1,0,0 }, { 255, 0, 0 }, 0, 0, 1.0f);
	debugManager_->addLine({ 0,0,0 }, { 0,1,0 }, { 0, 255, 0 }, 0, 0, 1.0f);
	debugManager_->addLine({ 0,0,0 }, { 0,0,1 }, { 0, 0, 255 }, 0, 0, 1.0f);
	glm::vec3 rot = { 0, 0, 0 };
	if (currentEntity_ && CurrentAnimationData_.animation)
	{
		drawHitboxes();
	}

	if (mouseHeld_ && currentObjectSelectedId_ != -1)
	{
		std::cout << "Currently selected object : " << currentObjectSelectedId_ << std::endl;
	}
}

void FighterCreatorTool::handleMouseScrolled(Events::MouseScrolledEvent& e)
{
	//if (e.scrollYOffset > 0)
	//{
	//	cameraDistanceFromFighter_ += CAMERA_DISTANCE_FROM_FIGHTER_DELTA;
	//}
	//else
	//{
	//	cameraDistanceFromFighter_ -= CAMERA_DISTANCE_FROM_FIGHTER_DELTA;
	//}

	//if (cameraDistanceFromFighter_ < CAMERA_DISTANCE_FROM_FIGHTER_THRESHOLD)
	//{
	//	cameraDistanceFromFighter_ = CAMERA_DISTANCE_FROM_FIGHTER_THRESHOLD;
	//}

	if (currentEntity_)
	{
		if (e.scrollYOffset > 0)
		{
			currentEntity_->getComponent<Transform>().scale_ *= 10;
		}
		else
		{
			currentEntity_->getComponent<Transform>().scale_ *= (.10);
		}
	}
	//lookAtCurrentEntity();
}

void FighterCreatorTool::handleMousePressedEvent(Events::MousePressedEvent& e)
{
	int uniqueId;
	if (e.mouseCode == 0 && debugManager_->getSelectedObject(uniqueId))
	{
		currentObjectSelectedId_ = uniqueId;
	}
	if (e.mouseCode == 0)
	{
		mouseHeld_ = true;
	}
}

void FighterCreatorTool::handleMouseReleasedEvent(Events::MouseReleasedEvent& e)
{
	//if (currentObjectSelectedId_ != -1)
	//{
	//	currentObjectSelectedId_ = -1;
	//	std::cout << "no object selected" << std::endl;
	//}
	if (e.mouseCode == 0)
	{
		mouseHeld_ = false;
	}
	if (e.mouseCode == 1 && currentObjectSelectedId_ != -1)
	{
		currentObjectSelectedId_ = -1;
	}
}

void findNextCameraPos(glm::vec3 currentPos)
{
	glm::vec3 center = { 0, 0, 0 };
	glm::vec3 newPos;
}

void FighterCreatorTool::handleMouseMovedEvent(Events::MouseMoveEvent& e)
{
	//move camera
	if (currentObjectSelectedId_ == -1 && currentEntity_ && mouseHeld_)
	{
		glm::vec2 currMousePos = { e.mouseX, e.mouseY };
		glm::vec2 delta = currMousePos - lastMousePos_;
		//Transform& transform = currentEntity_->getComponent<Transform>();
		//glm::mat4 currentRotation = glm::toMat4(transform.rot_);
		//transform.rot_ = glm::rotate(transform.rot_, delta.x * CAMERA_ROTATION_SPEED, { 0, 0, 1.0f });
		baseCamera_->position =  glm::rotate(delta.x * CAMERA_ROTATION_SPEED, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(delta.y * CAMERA_ROTATION_SPEED, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(baseCamera_->position, 1.0f);
		lookAtCurrentEntity();
	}

	//move hitbox selected case
	if (currentObjectSelectedId_ != -1 && mouseHeld_)
	{
		glm::vec2 currMousePos = { e.mouseX, e.mouseY };
		glm::vec2 delta = currMousePos - lastMousePos_;
		delta *= -MOVE_HITBOX_SENSITIVITY;
		Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
		if (hitbox)
		{
			glm::vec3 moveBy = { delta.x, 0.0f, delta.y };//camera is always facing in Y direction so only update x and y
			hitbox->pos_ += moveBy;
		}
	}
	lastMousePos_ = { e.mouseX, e.mouseY };
	debugManager_->mouseInfo_ = lastMousePos_;
}

void FighterCreatorTool::onEvent(Events::Event& e)
{
	if (!ui_->isMouseOverUI())
	{
		Events::EventDispatcher d(e);
		d.dispatch<Events::MouseScrolledEvent>(std::bind(&FighterCreatorTool::handleMouseScrolled, this, std::placeholders::_1));
		d.dispatch<Events::MousePressedEvent>(std::bind(&FighterCreatorTool::handleMousePressedEvent, this, std::placeholders::_1));
		d.dispatch<Events::MouseReleasedEvent>(std::bind(&FighterCreatorTool::handleMouseReleasedEvent, this, std::placeholders::_1));
		d.dispatch<Events::MouseMoveEvent>(std::bind(&FighterCreatorTool::handleMouseMovedEvent, this, std::placeholders::_1));
	}
}

Application* createApplication()
{
	return new FighterCreatorTool();
}

void FighterCreatorTool::DrawMenuBar()
{
	static bool errorLoadingMesh = false;
	static std::string error;

	static bool loadInMesh = false;
	static bool viewMeshes = false;
	static bool viewAnimations = false;
	static bool showMeshMenu = false;
	static bool showHitboxCreator = false;

	ui_->showImGuiDemoWindow();
	if (ui_->beginMainMenuBar())
	{
		if (ui_->beginMenu("Load"))
		{
			ui_->addMenuItem("Load in Mesh", &loadInMesh);
			ui_->addMenuItem("View Meshs", &viewMeshes);
			ui_->endMenu();
		}
		if (currentEntity_)
		{
			if (ui_->beginMenu("Tools"))
			{
				ui_->addMenuItem("View Animations", &viewAnimations);
				ui_->addMenuItem("View Hitbox Menu", &showMeshMenu);
				ui_->endMenu();
			}
		}
		ui_->endMainMenuBar();
	}

	if (loadInMesh)
	{
		ui_->beginWindow("Load in Mesh", 500, 500, &loadInMesh);
		ui_->addText("Input file location to model here");
		std::string input;
		if (ui_->addInput("input file path", &input))//true when enter is pressed
		{
			Entity* ent = NULL;
			if (!LoadMesh(input, error))
			{
				errorLoadingMesh = true;
				ui_->openPopup("ERROR");
			}
		}

		if (errorLoadingMesh)
		{
			bool isOpen = true;
			if (ui_->beginPopup("ERROR", NULL))
			{
				ui_->addText("ERROR: " + error);
				if (ui_->addButton("Close"))
				{
					std::cout << "closed" << std::endl;
					errorLoadingMesh = false;
					ui_->closePopup();
				}
				ui_->endPopup();
			}
		}

		ui_->EndWindow();
	}

	if (viewMeshes)
	{
		ui_->beginWindow("Load Mesh", 500, 500);
		//draw loaded meshes
		ui_->makeColumns("Loaded Meshs", 1);
		ui_->addText("Filepath");
		ui_->nextColumn();
		ui_->addSeperator();
		for (std::string filename : fileNames_)
		{
			bool selected = false;
			ui_->addText(filename);
			if (ui_->beginPopupContextItem("Options"))
			{
				if (ui_->addButton("LoadMesh") && currentEntity_ == NULL)
				{
					currentEntity_ = loadEntity(filename);
					lookAtCurrentEntity();
				}
				ui_->endPopup();
			}
			ui_->nextColumn();
		}
		if (currentEntity_)
		{
			ui_->addText("Set the rotation in degrees so that the mesh is facing the right");
			float rotateRight = 0.0f;
			if (ui_->addInputFloat("Right side rotation", rotateRight, 0.0f))
			{
				rotateRight *= AI_MATH_PI/180;
				rightSideRotation_ = rotateRight;
				rotateMeshRight();
			}
		}
		ui_->EndWindow();
	}

	if (viewAnimations && currentEntity_)
	{
		ui_->beginWindow("Animations", 500, 500);
		Animator& animator = currentEntity_->getComponent<Animator>();
		for (AnimationClip& animation : animator.animations_)
		{
			ui_->addText(animation.name_);
			if (ui_->beginPopupContextItem("Options"))
			{
				if (ui_->addButton("Select Animation"))
				{
					CurrentAnimationData_.animation = &animation;
					CurrentAnimationData_.hitboxData.resize(animation.frameCount_);
					CurrentAnimationData_.animation->isLooping_ = false;
				}
				ui_->endPopup();
			}
		}
		if (CurrentAnimationData_.animation)
		{
			if (ui_->addButton("Play"))
			{
				animator.setAnimation(CurrentAnimationData_.animation->name_);
				rotateMeshRight();
			}
			if (ui_->addButton("Stop"))
			{
				animator.setAnimation(-1);
			}
			ui_->addSlider("Frame selection", CurrentAnimationData_.currentFrame, 0, CurrentAnimationData_.getTotalAttackFrames());
			ui_->addInputInt("Select frame", CurrentAnimationData_.currentFrame, 1);
			if (CurrentAnimationData_.currentFrame < 0)
			{
				CurrentAnimationData_.currentFrame = 0;
			}
			else if (CurrentAnimationData_.currentFrame > CurrentAnimationData_.animation->frameCount_-1)
			{
				CurrentAnimationData_.currentFrame = CurrentAnimationData_.animation->frameCount_-1;
			}
			ui_->addInputInt("Startup", CurrentAnimationData_.startup, 1);
			ui_->addInputInt("Active", CurrentAnimationData_.active, 1);
			ui_->addInputInt("Recovery", CurrentAnimationData_.recovery, 1);
			ui_->addText("Select Attack input");
			if (ui_->beginPopupContextItem("Attack Input"))
			{
				if (ui_->addButton("Light"))
				{
					CurrentAnimationData_.input = Action::light;
				}

				if (ui_->addButton("Medium"))
				{
					CurrentAnimationData_.input = Action::medium;
				}

				if (ui_->addButton("Strong"))
				{

					CurrentAnimationData_.input = Action::strong;
				}

				if (ui_->addButton("Ultra"))
				{
					CurrentAnimationData_.input = Action::ultra;
				}
				ui_->endPopup();
			}
			char buff[100];
			sprintf_s(buff, "Current Attack Input: %d", CurrentAnimationData_.input);
			ui_->addText(buff);

			animator.getAnimationPoseByFrame(*CurrentAnimationData_.animation, 
			CurrentAnimationData_.currentFrame, currentEntity_->getComponent<Renderable>());

			if(ui_->addButton("Export Current Animation Data"))
			{
				exportCurrentAnimationData();
			}
		}
		ui_->EndWindow();
	}

	if (showMeshMenu && CurrentAnimationData_.animation)
	{
		ui_->beginWindow("Hitbox Menu", 500, 500);
		static Hitbox tempHitbox{};
		static float width;
		static float height;
		if (ui_->addInputFloat("Hitbox width", width, width))
		{
			tempHitbox.width_ = width;
		}
		if (ui_->addInputFloat("Hitbox height", height, height))
		{
			tempHitbox.height_ = height;
		}
		ui_->addText("Select hitbox layer");
		if (ui_->beginPopupContextItem("Hitbox Layer"))
		{
			if (ui_->addButton("Push"))
			{
				tempHitbox.layer_ = Hitbox::HitboxLayer::Push;
				tempHitbox.pos_ = { 0, 0, 0 }; //show up below other layers when looking down y axis
			}
			if (ui_->addButton("Hit") )
			{
				tempHitbox.layer_ = Hitbox::HitboxLayer::Hit;
				tempHitbox.pos_ = { 0, -0.01f, 0 }; //show up between push and hurt when looking down y axis
			}
			if (ui_->addButton("Hurt") && CurrentAnimationData_.currentFrame > CurrentAnimationData_.startup && CurrentAnimationData_.currentFrame < (CurrentAnimationData_.startup + CurrentAnimationData_.active))  //only add hitbot during active frames
			{
				tempHitbox.layer_ = Hitbox::HitboxLayer::Hurt;
				tempHitbox.pos_ = { 0, -0.02f, 0 }; //show up on top of all other layers when looking down y axis
			}
			ui_->endPopup();
		}
		if (ui_->addButton("Add hitbox"))
		{
			if (width != 0 && height != 0)
			{
				tempHitbox.uniqueId_ = createHitboxId();
				std::vector<Hitbox>& currentHitboxes = CurrentAnimationData_.getCurrentHitboxData();
				currentHitboxes.push_back(tempHitbox);
			}
		}
		if (ui_->addButton("Delete currently hitbox") && currentObjectSelectedId_ != -1)
		{
			Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
			std::vector<Hitbox>& currentHitboxes = CurrentAnimationData_.getCurrentHitboxData();
			for (int i = 0; i < currentHitboxes.size(); i++)
			{
				if (currentHitboxes[i].uniqueId_ == hitbox->uniqueId_)
				{
					currentHitboxes.erase(currentHitboxes.begin() + i);
					currentObjectSelectedId_ = -1;
				}
			}
		}
		if (ui_->addButton("Set current hitbox") && currentObjectSelectedId_ != -1)
		{
			Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
			if (hitbox)
			{
				hitbox->width_ = tempHitbox.width_;
				hitbox->height_ = tempHitbox.height_;
				hitbox->layer_ = tempHitbox.layer_;
			}
		}
		if (ui_->addButton("Copy Hitboxes from previous frame") &&  CurrentAnimationData_.currentFrame != 0)
		{
			std::vector<Hitbox>& hitboxData = CurrentAnimationData_.getCurrentHitboxData();
			std::vector<Hitbox> previousHitboxData = CurrentAnimationData_.getPreviousFrameHitboxData();
			hitboxData = previousHitboxData;
		}
		if(ui_->addButton("Copy currently selected hitbox to all frames") && currentObjectSelectedId_ != -1)
		{
			std::vector<Hitbox>& hitboxData = CurrentAnimationData_.getCurrentHitboxData();
			Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
			for (int i = 0; i < CurrentAnimationData_.hitboxData.size(); i++)
			{
				if (CurrentAnimationData_.currentFrame != i)
				{
					CurrentAnimationData_.hitboxData[i].push_back(*hitbox);
				}
			}
		}
		//if (ui_->addButton("Add child hitbox") && currentObjectSelectedId_ != -1)
		//{
		//	Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
		//	if (hitbox)
		//	{
		//		tempHitbox.uniqueId_ = createHitboxId();
		//		hitbox->children_.push_back(tempHitbox);
		//	}
		//}

		ui_->EndWindow();
	}
}

bool FighterCreatorTool::LoadMesh(const std::string& filePath, std::string& error)
{
	size_t found = filePath.find("dae", filePath.size() - 3);
	if (found == std::string::npos)
	{
		error = "Not a DAE file";
		return false;
	}

	if (!std::filesystem::exists(filePath))
	{
		error = "Could not find file";
		return false;
	}

	resourceManager_->loadAnimationFile(filePath);
	fileNames_.push_back(filePath);
	return true;
}

Entity* FighterCreatorTool::loadEntity(const std::string& filePath)
{
	const std::string DEFAULT_TEXTURE_PATH = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Textures\\missingTexture.jpg";

	AnimationReturnVals vals = resourceManager_->loadAnimationFile(filePath);
	TextureReturnVals texVals = resourceManager_->loadTextureFile(DEFAULT_TEXTURE_PATH);

	Entity* entity = scene_->addEntity(filePath);

	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, filePath);
	entity->addComponent<Renderable>(vals.vertices, vals.indices, false, filePath);
	entity->addComponent<Animator>(vals.animations, vals.boneStructIndex);
	Transform& transform = entity->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	transform.setScale(0.001f);
	transform.pos_ = { 0.0f, 0.0f, 0.0f };

	return entity;
}

bool FighterCreatorTool::lookAtCurrentEntity()
{
	if (!baseCamera_)
	{
		std::cout << "FighterCreatorTool::lookAtCurrentEntity(): baseCamera_ is null" << std::endl;;
		return false;
	}
	if (!currentEntity_)
	{
		std::cout << "FighterCreatorTool::lookAtCurrentEntity(): currentEntity_ is null" << std::endl;;
		return false;
	}

	Transform& transform = currentEntity_->getComponent<Transform>();
	glm::vec3 currPos = transform.pos_;
	//currPos.y += cameraDistanceFromFighter_;

	//baseCamera_->position = currPos;
	//baseCamera_->viewDirection = transform.pos_ - currPos;

	baseCamera_->viewDirection = transform.pos_ - baseCamera_->position;
	return true;
}

void FighterCreatorTool::rotateMeshRight()
{
	Transform& transform = currentEntity_->getComponent<Transform>();
	transform.rot_ = {0, 0, 0, 1};
	transform.rot_ = glm::rotate(transform.rot_, rightSideRotation_, { 0, 0, 1.0f });
}

void FighterCreatorTool::interpolateHitboxPosition(unsigned int uniqueHitboxId, unsigned int startFrame, unsigned int endFrame)
{
	if (endFrame < startFrame)
	{
		std::cout << "Error: interpolateHitboxPosition() endFrame smaller than startFrame" << std::endl;
		return;
	}
	for (int i = 0; i < (endFrame - startFrame); i++)
	{

	}
}

void FighterCreatorTool::exportCurrentAnimationData()
{
	ExportedAttack exportAttack{};
	exportAttack.startup = CurrentAnimationData_.startup;
	exportAttack.active = CurrentAnimationData_.active;
	exportAttack.recovery = CurrentAnimationData_.recovery;
	exportAttack.animationName = CurrentAnimationData_.animation->name_;
	exportAttack.input.push_back(CurrentAnimationData_.input);
	exportAttack.hitboxes = CurrentAnimationData_.hitboxData;
	//Add option to set these later
	exportAttack.blockStun = 1.0f; 
	exportAttack.blockStun = 1.0f;
	exportAttack.hitstun = 1.0f;
	exportAttack.damage = 1.0f;

	ExportData exportData{};
	exportData.attacks.push_back(exportAttack);
	exportData.modelFilePath = fileNames_[0];
	exportData.rightSideRotation = rightSideRotation_;
	exportData.textureFilePath = "C:\\Users\\jsngw\\source\\repos\\FightingGame\\FightingGameClient\\Textures\\missingTexture.jpg";

	AnimationDataFileExporter exporter(exportData);
}

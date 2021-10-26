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
	loadMesh("C:\\Users\\jsngw\\Downloads\\RobotKyle2.gltf", error);
	addEventCallback(std::bind(&FighterCreatorTool::onEvent, this, std::placeholders::_1));
}

void FighterCreatorTool::drawHitboxes()
{
	std::vector<Hitbox>& currentHitboxes = currentAnimationData_.getCurrentHitboxData();
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
	std::vector<Hitbox>& currentHitboxes = currentAnimationData_.getCurrentHitboxData();
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
	drawUI();
	debugManager_->addLine({ 0,0,0 }, { 1,0,0 }, { 255, 0, 0 });
	debugManager_->addLine({ 0,0,0 }, { 0,1,0 }, { 0, 255, 0 });
	debugManager_->addLine({ 0,0,0 }, { 0,0,1 }, { 0, 0, 255 });
	glm::vec3 rot = { 0, 0, 0 };
	if (currentEntity_ && currentAnimationData_.animation)
	{
		drawHitboxes();
	}
}

void FighterCreatorTool::handleMouseScrolled(Events::MouseScrolledEvent& e)
{

	if (currentEntity_)
	{
		if (e.scrollYOffset > 0)
		{
			currentEntity_->getComponent<Transform>().scale_ *= 1.5;
		}
		else
		{
			currentEntity_->getComponent<Transform>().scale_ *= (.667);
		}
	}
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
		baseCamera_->position =  glm::rotate(delta.x * CAMERA_ROTATION_SPEED, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(delta.y * CAMERA_ROTATION_SPEED, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(baseCamera_->position, 1.0f);
		lookAtCurrentEntity();
	}

	//move selected hitbox
	if (currentObjectSelectedId_ != -1 && mouseHeld_)
	{
		glm::vec2 currMousePos = { e.mouseX, e.mouseY };
		glm::vec2 delta = currMousePos - lastMousePos_;
		delta *= -MOVE_HITBOX_SENSITIVITY;
		Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
		if (hitbox)
		{
			glm::vec3 moveBy = { 0.0f, delta.x, delta.y };//camera is always facing in Y direction so only update x and y
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

void FighterCreatorTool::drawMeshMenus()
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
		ui_->addText("Make the fighter face towards the +Y axis(Green line)");
		float rotateRight = 0.0f;
		if (ui_->addInputFloat("Right side rotation", rotateRight, 0.0f))
		{
			rotateRight *= AI_MATH_PI/180;
			rightSideRotation_ = rotateRight;
			rotateMeshRight();
		}
		float rotateUp = 0.0f;
		ui_->addText("Orient the fighter so its standing up facing the +Z axis(Blue line)");
		if (ui_->addInputFloat("up side rotation", rotateUp, 0.0f))
		{
			rotateUp *= AI_MATH_PI / 180;
			upRotation_ = rotateUp;
			rotateMeshRight();
		}

		if(ui_->addButton("Export Current Animation Data"))
		{
			exportCurrentAnimationData();
		}
	}
	ui_->EndWindow();
}

void FighterCreatorTool::drawUI()
{
	static bool errorLoadingMesh = false;
	static std::string error;

	static bool loadInMesh = false;
	static bool viewMeshes = false;
	static bool viewAnimations = false;
	static bool showMeshMenu = false;
	static bool showHitboxCreator = false;

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
		ui_->beginWindow("Load in Mesh and Texture Data", 500, 500, &loadInMesh);
		ui_->addText("Input file location to model here");
		std::string meshInput;
		if (ui_->addInput("input file path", &meshInput))//true when enter is pressed
		{
			Entity* ent = NULL;
			if (!loadMesh(meshInput, error))
			{
				errorLoadingMesh = true;
				ui_->openPopup("ERROR");
			}
		}

		ui_->addText("Set Texture before loading mesh");
		std::string textureInput;
		if (ui_->addInput("Texture Path", &textureInput))//true when enter is pressed
		{
			if (std::filesystem::exists(textureInput))
			{
				texturePath_ = textureInput;
			}
		}


		if (errorLoadingMesh)
		{
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
		drawMeshMenus();
	}

	if (viewAnimations && currentEntity_)
	{
		ui_->beginWindow("Animations", 500, 500);
		Animator& animator = currentEntity_->getComponent<Animator>();
		for (AnimationClip& animation : animator.animations_)
		{
			if (ui_->addSelectable(animation.name_, false))
			{
				currentAnimationData_.animation = &animation;
				currentAnimationData_.hitboxData.resize(animation.frameCount_);
				currentAnimationData_.animation->isLooping_ = false;
			}
		}
		if (currentAnimationData_.animation)
		{
			ui_->addSlider("Frame Slider", currentAnimationData_.currentFrame, 0, currentAnimationData_.animation->frameCount_-1);
			ui_->addInputInt("Select frame", currentAnimationData_.currentFrame, 1);
			if (currentAnimationData_.currentFrame < 0)
			{
				currentAnimationData_.currentFrame = 0;
			}
			else if (currentAnimationData_.currentFrame > currentAnimationData_.animation->frameCount_-1)
			{
				currentAnimationData_.currentFrame = currentAnimationData_.animation->frameCount_-1;
			}
			ui_->addInputInt("Startup", currentAnimationData_.startup, 1);
			ui_->addInputInt("Active", currentAnimationData_.active, 1);
			ui_->addInputInt("Recovery", currentAnimationData_.recovery, 1);
			ui_->addInputFloat("BlockStun", currentAnimationData_.blockStun, 1);
			ui_->addInputFloat("Hitstun", currentAnimationData_.hitstun, 1);
			ui_->addInputFloat("Damage", currentAnimationData_.damage, 1);
			ui_->addText("Select Attack input");
			if (ui_->beginPopupContextItem("Attack Input"))
			{
				if (ui_->addButton("Light"))
				{
					currentAnimationData_.input = Action::light;
				}

				if (ui_->addButton("Medium"))
				{
					currentAnimationData_.input = Action::medium;
				}

				if (ui_->addButton("Strong"))
				{

					currentAnimationData_.input = Action::strong;
				}

				if (ui_->addButton("Ultra"))
				{
					currentAnimationData_.input = Action::ultra;
				}
				ui_->endPopup();
			}
			ui_->addText("Select attack type");
			if (ui_->beginPopupContextItem("Attack type"))
			{
				if (ui_->addButton("Standing"))
				{
					currentAnimationData_.attackType = Standing;
				}
				if (ui_->addButton("Crouching"))
				{
					currentAnimationData_.attackType = Crouching;
				}
				if (ui_->addButton("Jumping"))
				{
					currentAnimationData_.attackType = Jumping;
				}
				ui_->endPopup();
			}
			char buff[100];
			sprintf_s(buff, "Current Attack Input: %d", currentAnimationData_.input);
			ui_->addText(buff);

			animator.getAnimationPoseByFrame(*currentAnimationData_.animation, 
			currentAnimationData_.currentFrame, currentEntity_->getComponent<Renderable>());

		}
		ui_->EndWindow();
	}

	if (showMeshMenu && currentAnimationData_.animation)
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
				tempHitbox.pos_ = { -0.01f, 0, 0 }; //show up between push and hurt when looking down y axis
			}
			if (currentAnimationData_.currentFrame >= currentAnimationData_.startup && currentAnimationData_.currentFrame < (currentAnimationData_.startup + currentAnimationData_.active) && ui_->addButton("Hurt") )  //only add hitbot during active frames
			{
				tempHitbox.layer_ = Hitbox::HitboxLayer::Hurt;
				tempHitbox.pos_ = { -0.02f, 0, 0 }; //show up on top of all other layers when looking down y axis
			}
			ui_->endPopup();
		}
		if (ui_->addButton("Add hitbox"))
		{
			if (width != 0 && height != 0)
			{
				tempHitbox.uniqueId_ = createHitboxId();
				std::vector<Hitbox>& currentHitboxes = currentAnimationData_.getCurrentHitboxData();
				currentHitboxes.push_back(tempHitbox);
			}
		}
		if (ui_->addButton("Delete current hitbox") && currentObjectSelectedId_ != -1)
		{
			Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
			std::vector<Hitbox>& currentHitboxes = currentAnimationData_.getCurrentHitboxData();
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
		if (ui_->addButton("Copy Hitboxes from previous frame") &&  currentAnimationData_.currentFrame != 0)
		{
			std::vector<Hitbox>& hitboxData = currentAnimationData_.getCurrentHitboxData();
			std::vector<Hitbox> previousHitboxData = currentAnimationData_.getPreviousFrameHitboxData();
			hitboxData = previousHitboxData;
		}
		if(ui_->addButton("Copy currently selected hitbox to all frames") && currentObjectSelectedId_ != -1)
		{
			std::vector<Hitbox>& hitboxData = currentAnimationData_.getCurrentHitboxData();
			Hitbox* hitbox = getHitboxById(currentObjectSelectedId_);
			for (int i = 0; i < currentAnimationData_.hitboxData.size(); i++)
			{
				if (currentAnimationData_.currentFrame != i ) 	
				{
					switch (hitbox->layer_)
					{
					case Hitbox::HitboxLayer::Hurt://dont copy hurtboxes into frames that aren't active
						if (currentAnimationData_.isFrameInActive(i))
						{
							currentAnimationData_.hitboxData[i].push_back(*hitbox);
						}
						break;
					default:
						currentAnimationData_.hitboxData[i].push_back(*hitbox);
						break;
					}
				}
			}
		}
		static bool selectingAnimationType = false;
		if (ui_->addButton("Done with animation"))
		{
			ui_->openPopup("Select Animation Type");
			selectingAnimationType = true;
		}
		if (selectingAnimationType)
		{
			if (ui_->beginPopup("Select Animation Type", NULL))
			{
				ui_->addText("Select the type of animation\n(Walking Forward/Backward, idle, attack)");
				if (ui_->addButton("Walking Animation"))
				{
					walkForwardAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				if (ui_->addButton("Idle Animation"))
				{

					idleAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				if (ui_->addButton("Jump Animation"))
				{

					jumpAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				if (ui_->addButton("Hit Animation"))
				{

					hitAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				if (ui_->addButton("Block Animation"))
				{
					blockAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				if (ui_->addButton("Crouch Animation"))
				{
					crouchAnimationData_ = currentAnimationData_;
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}

				if (ui_->addButton("Attack Animation"))
				{
					attackAnimationData_.push_back(currentAnimationData_);
					currentAnimationData_ = {};
					currentObjectSelectedId_ = -1;
					selectingAnimationType = false;
				}
				ui_->endPopup();
			}
		}
		ui_->EndWindow();
	}
}

bool FighterCreatorTool::loadMesh(const std::string& filePath, std::string& error)
{
	//size_t found = filePath.find("dae", filePath.size() - 3);
	//if (found == std::string::npos)
	//{
	//	error = "Not a DAE file";
	//	return false;
	//}

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

	if (texturePath_.size() == 0)
	{
		texturePath_ = DEFAULT_TEXTURE_PATH;
	}

	AnimationReturnVals vals = resourceManager_->loadAnimationFile(filePath);
	TextureReturnVals texVals = resourceManager_->loadTextureFile(texturePath_);

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

	baseCamera_->viewDirection = transform.pos_ - baseCamera_->position;
	return true;
}

void FighterCreatorTool::rotateMeshRight()
{
	Transform& transform = currentEntity_->getComponent<Transform>();
	transform.rot_ = {0, 0, 0, 1};
	transform.rot_ = glm::rotate(transform.rot_, rightSideRotation_, { 0, 0, 1.0f });
	transform.rot_ = glm::rotate(transform.rot_, upRotation_, { 1.0f, 0.0, 0.0 });
}

void FighterCreatorTool::exportCurrentAnimationData()
{
	ExportData exportData{};
	for (const AnimationData& animationData : attackAnimationData_)
	{
		ExportedAttack exportAttack{};
		exportAttack.startup = animationData.startup;
		exportAttack.active = animationData.active;
		exportAttack.recovery = animationData.recovery;
		exportAttack.animationName = animationData.animation->name_;
		exportAttack.input.push_back(animationData.input);
		exportAttack.hitboxes = animationData.hitboxData;
		//Add option to set these later
		exportAttack.blockStun = 1.0f; 
		exportAttack.hitstun = 1.0f;
		exportAttack.damage = 1.0f;
		exportAttack.type = animationData.attackType;
		exportData.attacks.push_back(exportAttack);
	}
	exportData.modelFilePath = fileNames_[0];
	exportData.idleData.animationName = idleAnimationData_.animation->name_;
	exportData.idleData.hitboxData = idleAnimationData_.hitboxData;
	exportData.walkData.animationName = walkForwardAnimationData_.animation->name_;
	exportData.walkData.hitboxData = walkForwardAnimationData_.hitboxData;
	exportData.jumpData.hitboxData = jumpAnimationData_.hitboxData;
	exportData.jumpData.animationName = jumpAnimationData_.animation->name_;
	exportData.crouchData.hitboxData = crouchAnimationData_.hitboxData;
	exportData.crouchData.animationName = crouchAnimationData_.animation->name_;
	exportData.hitData.hitboxData = hitAnimationData_.hitboxData;
	exportData.hitData.animationName = hitAnimationData_.animation->name_;
	exportData.blockData.hitboxData = exportData.hitData.hitboxData;
	exportData.blockData.animationName = exportData.hitData.animationName;
	exportData.rightSideRotation = rightSideRotation_;
	exportData.textureFilePath = texturePath_;

	FighterFileExporter exporter(exportData);
}

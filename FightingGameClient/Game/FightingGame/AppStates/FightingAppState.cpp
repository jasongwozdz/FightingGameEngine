#include "FightingAppState.h"
#include "ResourceManager.h"
#include "Scene/Components/Camera.h"
#include "Scene/Behaviors/ControllableCameraBehavior.h"
#include "../Arena.h"
#include "../ArenaFloorBehavior.h"
#include "../ArenaWallBehavior.h"
#include "Input.h"
#include "../FighterSandbox/AttackTypes.h"
#include "Console/Console.h"
#include "Particles/ParticleManager.h"

FightingAppState::FightingAppMode FightingAppState::mode_ = FightingAppState::NORMAL;

FightingAppState::FightingAppState(std::string fighter1, std::string fighter2, DebugDrawManager* debugDrawManager, InputHandler* inputLeft, InputHandler* inputRight) : 
	scene_(Scene::getSingletonPtr()),
	resourceManager_(ResourceManager::getSingletonPtr()),
	inputHandlerLeft_(inputLeft),
	inputHandlerRight_(inputRight),
	debugDrawManager_(debugDrawManager)
{
	//scene_->setSkybox("C:/Users/jsngw/source/repos/FightingGame/FightingGameClient/Textures/skybox");
	fighter1 = "..\\FighterFiles\\NewFighter\\BasicFighter.fgAnim";
	fighter2 = "..\\FighterFiles\\NewFighter\\BasicFighter.fgAnim";
	inputHandlerLeft_->clearInputQueue();
	inputHandlerRight_->clearInputQueue();
	fighterFactory_ = new FighterFactory(*scene_);
	initScene(fighter1, fighter2);
	initArena();
	initCamera();
	initColliderLayers();
	initConsoleCommands();
	Input::getSingleton().addButton("Escape", GLFW_KEY_ESCAPE);
	gameStateManager_ = new GameStateManager(fighters_[0], fighters_[1], debugDrawManager, *arena_);
}

FightingAppState::~FightingAppState()
{
	delete inputHandlerLeft_;
	delete inputHandlerRight_;
	delete fighterFactory_;
	delete arena_;
}

void FightingAppState::enterState()
{}

void FightingAppState::initScene(std::string fighterFilePath1, std::string fighterFilePath2)
{
	const glm::vec3 STARTING_POSITION_LEFT =  { 5.0f, 0.0f, 0.0f};
	const glm::vec3 STARTING_POSITION_RIGHT = { -5.0f, 0.0f, 0.0f};

	fighters_.resize(2);
	fighters_[0] = fighterFactory_->createFighterNew(fighterFilePath1, *inputHandlerLeft_);
	fighters_[1] = fighterFactory_->createFighterNew(fighterFilePath2, *inputHandlerRight_);

	Transform& transform1 = fighters_[0]->getComponent<Transform>();
	Transform& transform2 = fighters_[1]->getComponent<Transform>();
	transform1.position_ = STARTING_POSITION_LEFT;
	transform2.position_ = STARTING_POSITION_RIGHT;

	Fighter* fighterLeft = Fighter::getFighterComp(fighters_[0]);
	fighterLeft->side_ = FighterSide::left;

	glm::vec3 pos1 = transform1.position_;
	glm::vec3 pos2 = transform2.position_;

	transform1.lookAt(pos2 - pos1);
	transform2.lookAt(pos1 - pos2);

	Entity* dirLightEntity = scene_->addEntity("DirLight");
	dirLightEntity->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	DirLight& dirLight = dirLightEntity->addComponent<DirLight>(dirLightEntity);

	Entity* pointLightEntity = scene_->addEntity("PointLight");
	pointLightEntity->addComponent<Transform>(0.0f, 1.0f, 0.0f);
	PointLight& pointLight = pointLightEntity->addComponent<PointLight>(pointLightEntity);
	pointLight.uniformData_.ambient = { 0.05f, 0.05f, 0.05f };
	pointLight.uniformData_.diffuse = { 1.0f, 1.0f, 1.0f };
	pointLight.uniformData_.specular = { 1.0f, 1.0f, 1.0f };

	Entity* backgroundEntity = scene_->addEntity("Background");
	backgroundEntity->addComponent<Transform>(0.0f, 0.0f, 0.0f);
	AssetCreateInfo createInfo;
	createInfo.modelPath = "Models/Background.obj";
	//createInfo.texturePath = "Models/Shop_texturasdasdasde.png";
	Asset* asset = resourceManager_->createAsset(createInfo);
	backgroundEntity->addComponent<AssetInstance>(asset);

	ParticleManager* particleManager = ParticleManager::getSingletonPtr();
	//CreateParticleInfo particleInfo;
	//particleInfo.lifeTime = 1.0f;
	//particleInfo.velocity = { 0.005f, 0.005f, 0.0f };
	//particleInfo.size = 0.5f;
	//particleInfo.startingPos = { 0.0f, 0.0f, 0.0f };
	//particleInfo.degreesPerFrame = 1.0f;
	//particleInfo.texturePath = "Textures/missingTexture.jpg";
	//particleManager->addParticle(particleInfo);

	CreateParticleEmitter emitter;
	emitter.rate = 1;
	emitter.velocity = { 0.1f, 1.0f };
	emitter.scale = { 0.5f, 1.0f };
	emitter.lifeTime = {10.0f, 10.0f};
	emitter.angularVel = { 0.5f, 0.5f };

	particleManager->createParticleEmitter(emitter);
	generateArenaBackground();
}

void FightingAppState::initCamera()
{
	fightingGameCamera_ = scene_->createCameraEntity();
	fightingGameCamera_->addComponent<Behavior>(new FightingAppCameraBehavior(fightingGameCamera_, fighters_));
	scene_->setActiveCamera(fightingGameCamera_);

	flyCamera_ = scene_->createCameraEntity();
	flyCamera_->addComponent<Behavior>(new ControllableCameraBehavior(flyCamera_));
}

void FightingAppState::initColliderLayers()
{
	BoxCollisionManager* boxCollisionManager = BoxCollisionManager::getSingletonPtr();
	boxCollisionManager->addLayerRule(ColliderLayer::PUSH_BOX, ColliderLayer::PUSH_BOX);
	boxCollisionManager->addLayerRule(ColliderLayer::HURT_BOX, ColliderLayer::HIT_BOX);
}

void FightingAppState::checkInput()
{
	Input& input = Input::getSingleton();
	if (input.getButton("Fire3") > 0.0f)
	{
		scene_->setActiveCamera(flyCamera_);
	}
	else if (input.getButton("Fire2") > 0.0f)
	{
		scene_->setActiveCamera(fightingGameCamera_);
	}
	else if (input.getButton("Escape") > 0.0f)
	{
		mode_ = (FightingAppState::FightingAppMode)((int)mode_ ^ 1);//swap mode
	}
}

void FightingAppState::initArena()
{
	arena_ = new Arena();
	arena_->pos = glm::vec3(0.0f);
	arena_->size = glm::vec3(21.0f, 1.0f, 21.0f);
	arena_->entity = scene_->addEntity("Arena");
	arena_->entity->addComponent<Transform>(arena_->pos);
	arena_->entity->addComponent<Behavior>(new ArenaFloorBehavior(arena_->entity, *arena_));
	//arena_->entity->addComponent<Collider>(arena_->entity, arena_->size, glm::vec3(0.0f, 0.0f, 0.0f), 0);

	auto createWallEntity = [&](ArenaWallBehavior::ArenaWallSide side)
	{
		Entity* entity = scene_->addEntity("ArenaWall");
		entity->addComponent<Transform>(0.0f, 0.0f, 0.0f);
		glm::vec3 size = glm::vec3(arena_->size.y, arena_->size.x, arena_->size.z);
		//Collider collider = Collider(entity, size, glm::vec3(0.0f), 0);
		//entity->addComponent<Collider>(collider);
		entity->addComponent<Behavior>(new ArenaWallBehavior(entity, *arena_, side));
		return entity;
	};

	arena_->walls.push_back(createWallEntity(ArenaWallBehavior::ArenaWallSide::LEFT));
	arena_->walls.push_back(createWallEntity(ArenaWallBehavior::ArenaWallSide::RIGHT));
}

void FightingAppState::generateArenaBackground()
{
	//const glm::vec3 BACKGROUND_POS = { 16.0f, 11.0f, -1.5f };//set position of background here
	//
	//const std::string arenaBackground = "./Models/WaverlyTownHall-02_1mil_meters_sf.obj";
	//const std::string arenaBackgroundTexturePath = "./Textures/WaverlyTownHall-02_1mil_meters_sf.jpg";

	//arena_.entity = scene_->addEntity("ArenaBackground");
	//const ModelReturnVals& backgroundModelData = resourceManager_->loadObjFile(arenaBackground);
	//const TextureReturnVals& textureReturnVals = resourceManager_->loadTextureFile(arenaBackgroundTexturePath);
	//arena_.entity->addComponent<Renderable>(backgroundModelData.vertices, backgroundModelData.indices, false, "ArenaBackground");
	//arena_.entity->addComponent<Textured>(textureReturnVals.pixels, textureReturnVals.textureWidth, textureReturnVals.textureHeight, textureReturnVals.textureChannels, "ArenaBackground");
	//Transform& arenaTransform = arena_.entity->addComponent<Transform>(BACKGROUND_POS.x, BACKGROUND_POS.y, BACKGROUND_POS.z);
}

bool FightingAppState::isFrameByFrameModeActive()
{
	return mode_ == FightingAppMode::FRAMEBYFRAME;
}

AppState* FightingAppState::update(float deltaTime)
{
	gameStateManager_->update(deltaTime);
	checkInput();
	return nullptr;
}

void FightingAppState::consoleCommandCallback(std::string command, CommandVar* commandVar)
{
	std::vector<Entity*> dirLights;
	scene_->getEntities("DirLight", dirLights);
	for (auto iter : dirLights)
	{
		DirLight& dirLight = iter->getComponent<DirLight>();
		if (command == "dirlight.ambient")
		{
			dirLight.uniformData_.ambient = commandVar->data.vec3Data;
		}
		else if (command == "dirlight.diffuse")
		{
			dirLight.uniformData_.diffuse = commandVar->data.vec3Data;
		}
		else if (command == "dirlight.direction")
		{
			dirLight.uniformData_.direction = commandVar->data.vec3Data;
		}
	}

	std::vector<Entity*> pointLights;
	scene_->getEntities("PointLight", pointLights);
	for (auto iter : pointLights)
	{
		PointLight& pointLight = iter->getComponent<PointLight>();
		Transform& transform = iter->getComponent<Transform>();
		if (command == "pointLight.ambient")
		{
			pointLight.uniformData_.ambient = commandVar->data.vec3Data;
		}
		else if (command == "pointLight.diffuse")
		{
			pointLight.uniformData_.diffuse = commandVar->data.vec3Data;
		}
		else if (command == "pointLight.position")
		{
			transform.position_ = commandVar->data.vec3Data;
		}
	}
}

void FightingAppState::initConsoleCommands()
{
	Console* console = Console::getInstance();
	console->addVec3Var("dirlight.ambient", { 1.0f,1.0f,1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));
	console->addVec3Var("dirlight.diffuse", { 1.0f, 1.0f, 1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));
	console->addVec3Var("dirlight.direction", { 1.0f, 1.0f, 1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));


	console->addVec3Var("pointLight.ambient", { 1.0f,1.0f,1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));
	console->addVec3Var("pointLight.diffuse", { 1.0f,1.0f,1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));
	console->addVec3Var("pointLight.position", { 1.0f,1.0f,1.0f }, std::bind(&FightingAppState::consoleCommandCallback, this, std::placeholders::_1, std::placeholders::_2));
}

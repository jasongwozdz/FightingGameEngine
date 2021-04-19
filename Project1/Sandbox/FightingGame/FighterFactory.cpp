#include "FighterFactory.h"
#include "ResourceManager.h"

FighterFactory::FighterFactory(Scene& scene, InputHandler& inputHandler) :
	scene_(scene),
	inputHandler_(inputHandler)
{}

Fighter* FighterFactory::createFighter(const std::string& modelPath, const std::string& texturePath, InputHandler& inputHandler)
{
	Entity* entity = scene_.addEntity("Fighter");
	ResourceManager& resourceManager = ResourceManager::getSingleton();
	AnimationReturnVals ret = resourceManager.loadAnimationFile((std::string&)modelPath);
	TextureReturnVals texVals = resourceManager.loadTextureFile((std::string&)texturePath);

	entity->addComponent<Textured>(texVals.pixels, texVals.textureWidth, texVals.textureHeight, texVals.textureChannels, "Fighter");
	entity->addComponent<Renderable>(ret.vertices, ret.indices, false, "Fighter");
	Transform& transform = entity->addComponent<Transform>( 1.0f, 1.0f, 1.0f );
	transform.setScale(0.001f);

	Animator& animator = entity->addComponent<Animator>(ret.animations, ret.boneStructIndex);
	animator.setAnimation(-1);

	float HITBOX_WIDTH = 3.5;
	float HITBOX_HEIGHT = 5;
	glm::vec3 HITBOX_POS = { 0.0f, 0.0f, 0.0f };

	Hitbox& hitbox = entity->addComponent<Hitbox>(HITBOX_WIDTH, HITBOX_HEIGHT, HITBOX_POS);

	//Fighter* fighter = new Fighter(entity, inputHandler_);
	Fighter* fighter = new Fighter(entity, inputHandler);
	return fighter;
}

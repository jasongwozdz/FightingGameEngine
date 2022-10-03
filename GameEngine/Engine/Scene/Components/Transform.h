#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../Renderer/Renderable.h"
#include "../Entity.h"

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

class AssetInstance;

class ENGINE_API Transform 
{
public:
	Transform(float x, float y, float z);

	Transform(glm::vec3 pos);

	void setScale(float scale);

	static glm::vec3 worldUp;
	static glm::vec3 worldLeft;
	static glm::vec3 worldForward;

	//check what side vector checkSide is in relation to middle
	// > 0 indicates right, < 0 indicates left
	static float checkSide(glm::vec2 middle, glm::vec2 checkSide);

	glm::vec3 left() const;
	glm::vec3 up() const;
	glm::vec3 forward() const;

	void drawDebugGui();
	void drawDebug();

	glm::mat4 calculateTransform();
	glm::mat4 calculateTransformNoScale();
	void applyTransformToMesh(Renderable& mesh);
	void applyTransformToMesh(AssetInstance* assetInstance);

	//direction to point forward vector at
	void lookAt(glm::vec3 direction);
	//rotation around each local axis
	void rotateAround(glm::vec3 rotationInRadians);
	void rotateAround(float angleToRotateInDegrees, glm::vec3 axisToRotateAround);
	void moveTowards(glm::vec3 finalPos, float maxMoveMag);

public:
	glm::mat4 finalTransform_ = glm::mat4(1.0f);

	glm::vec3 position_ = {0.0f, 0.0f, 0.0f};

	glm::vec3 scale_ = {1.0f, 1.0f, 1.0f};

	glm::quat rotation_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	bool drawDebugGui_ = false;
	bool drawDebug_ = false;

private:
	glm::vec3 oldPos_;
	glm::vec3 oldScale_;
	glm::quat oldRot_;
	bool calculateTransform_ = true;
};

#pragma once
#include <fstream>
#include "Fighter.h"

class FighterFileImporter
{
public:
	FighterFileImporter(const std::string& filePath);
	~FighterFileImporter();

	struct AnimationData {
		std::string animationName;
		std::vector<std::vector<Hitbox>> hitboxData;
	};

	struct {
		std::string modelFilePath;
		std::string textureFilePath;
		float rightSideRotation; // in radians 
		float upRotation; // in radians 
		AnimationData idleData;
		AnimationData walkingData;
		std::vector<std::vector<InputKey>> inputData;
		std::vector<Attack> attacks;
		std::vector<AnimationData> attackData;
	} exportData_;

private:
	void readFile();
	std::vector<std::vector<Hitbox>> extractHitboxData(std::string hitboxData);
	std::ifstream file_;//opened in constructor closed in destructor
};


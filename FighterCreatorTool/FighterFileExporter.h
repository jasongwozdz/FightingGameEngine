#pragma once
#include <fstream>
#include <vector>

#include "Hitbox.h"

typedef uint8_t InputKey;

enum Action
{
	nothing = 0,
	left = 1, // 1 << 0
	right = 2, // 1 << 1
	down = 4, // 1 << 2
	up  = 8, // 1 << 3
	light = 16, // 1 << 4
	medium = 32, // 1 << 5
	strong = 64,// 1 << 6
	ultra = 128, // 1 << 7
};

struct ExportedAttack
{
	unsigned int startup;
	unsigned int active;
	unsigned int recovery;
	unsigned int blockStun;
	unsigned int hitstun;
	int type;
	float pushMag;
	unsigned int damage;
	std::string animationName;
	std::vector<uint8_t> input;
	std::vector<std::vector<Hitbox>> hitboxes;
};

struct AnimationData
{
	std::string animationName;
	std::vector<std::vector<Hitbox>> hitboxData;
};

struct ExportData{
	std::string modelFilePath;
	std::string textureFilePath;
	float rightSideRotation; // in radians 
	float upRotation; // in radians
	AnimationData idleData;
	AnimationData walkData;
	AnimationData jumpData;
	AnimationData hitData;
	AnimationData crouchData;
	AnimationData blockData;
	std::vector<ExportedAttack> attacks;
};

class FighterFileExporter
{
public:
	FighterFileExporter(const ExportData& exportData);
	~FighterFileExporter();
private:
	void AddHitboxToFile(const std::vector<Hitbox>& hitbox);
	void populateFile(const ExportData& exportData);
	void addAnimationDataToFile(const std::string& AnimationName,  const AnimationData& animationData);
private:
	std::ofstream file_;//opened in constructor closed in destructor
	const std::string& FOLDER_LOCATION = "../FighterFiles/";
};


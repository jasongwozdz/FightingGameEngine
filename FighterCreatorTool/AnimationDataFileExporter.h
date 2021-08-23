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
	unsigned int damage;
	std::string animationName;
	std::vector<uint8_t> input;
	std::vector<std::vector<Hitbox>> hitboxes;
};

struct ExportData{
	std::string modelFilePath;
	std::string textureFilePath;
	float rightSideRotation; // in radians 
	std::vector<ExportedAttack> attacks;
};

class AnimationDataFileExporter
{
public:
	AnimationDataFileExporter(const ExportData& exportData);
	~AnimationDataFileExporter();
private:
	void AddHitboxToFile(const std::vector<Hitbox>& hitbox);
	std::ofstream file_;//opened in constructor closed in destructor
};


#pragma once
#include <fstream>
#include "Fighter.h"

class FighterFileImporter
{
public:
	FighterFileImporter(const std::string& filePath);
	~FighterFileImporter();

	struct AttackData {
		std::vector<std::vector<Hitbox>> hitboxData;
	};

	struct {
		std::string modelFilePath;
		std::string textureFilePath;
		float rightSideRotation; // in radians 
		std::vector<Attack> attacks;
		std::vector<AttackData> attackData;
	} exportData_;

private:
	void readFile();
	std::ifstream file_;//opened in constructor closed in destructor
};


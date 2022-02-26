#pragma once
#include <fstream>

#include "Fighter/Fighter.h"
#include "Scene/Components/Collider.h"

class FighterFileImporter
{
public:
	FighterFileImporter(const std::string& filePath);
	~FighterFileImporter();

	struct AnimationData 
	{
		std::string animationName;
		std::vector<FrameInfo> frameData;
	};

	struct AttackData 
	{
		std::vector<uint8_t> inputs_;
		std::vector<FrameInfo> frameData_;
		std::string animationName_;
		int startupFrames_;
		int activeFrames_;
		int recoveryFrames_;
		int blockstun_;
		int hitstun_;
		int freezeFrames_;
		float push_;
		float damage_;
	};

	struct {
		std::string modelFilePath;
		std::string textureFilePath;
		std::vector<AttackData> attackData;

		std::vector<MoveInfo> moveData;
		std::vector<std::vector<uint8_t>> inputs;

		BoxCollider basePushBox;
		AnimationData idleData;
		AnimationData walkingData;
		AnimationData crouchData;
		AnimationData jumpData;
		AnimationData hitData;
		AnimationData blockData;
	} exportData_;

private:
	void readFile();
	void readFileNew(std::string filepath);
	MoveInfo extractAttackDataFile(std::string filePath);
	std::vector<FrameInfo> extraAttackColliderData(std::string hitboxData);
	std::vector<uint8_t> extractInput(std::string inputData);
	std::string extractSingleParen(std::string line, std::string descriptor);
	float extractSingleParenInt(std::string line, std::string descriptor);
	glm::vec3 extractTripleParenVec(std::string line, std::string descriptor);
	std::vector<std::string> extractParenStr(std::string line, std::string descriptor);
	std::vector<int> extractParenInt(std::string line, std::string descriptor);
	BoxCollider extractColliderData(std::ifstream& file);
	int extractTags(std::string line);

private:
	std::ifstream file_;//opened in constructor closed in destructor
	bool flipHitboxes_ = false;
};


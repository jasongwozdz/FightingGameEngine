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
		BoxCollider basePushBox;
		float rightSideRotation; // in radians 
		float upRotation; // in radians 
		AnimationData idleData;
		AnimationData walkingData;
		AnimationData crouchData;
		AnimationData jumpData;
		AnimationData hitData;
		AnimationData blockData;
	} exportData_;

private:
	void readFileNew();
	std::vector<FrameInfo> extraAttackColliderData(std::string hitboxData);
	std::vector<uint8_t> extractInput(std::string inputData);

private:
	std::ifstream file_;//opened in constructor closed in destructor
	bool flipHitboxes_ = false;
};


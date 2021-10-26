#include <iostream>
#include <filesystem>

#include "FighterFileExporter.h"

#define ADD_TO_FILE(STRINGNAME) \
	file_ << STRINGNAME << " : "

FighterFileExporter::FighterFileExporter(const ExportData& exportData)
{
	std::filesystem::create_directory(FOLDER_LOCATION);//create folder if it doesn't exist

	char fileName[500];
	sprintf_s(fileName, "%s%s.fgAnim", FOLDER_LOCATION.c_str(), exportData.attacks[0].animationName.c_str());
	file_.open (fileName, std::ofstream::out);
	if(file_.is_open())
	{
		populateFile(exportData);
	}
	else
	{
		std::cout << "cant open file" << std::endl;
	}
}

FighterFileExporter::~FighterFileExporter()
{
	file_.close();
}

//Store hitboxes in this format [{width,height,pos.x,pos.y,pos.z,layer}{...}....]
void FighterFileExporter::AddHitboxToFile(const std::vector<Hitbox>& hitboxes)
{
	file_ << '[';//start of hitbox data for frame
	for (const Hitbox& hitbox : hitboxes)
	{
		file_ << "{" << hitbox.width_ << "," << hitbox.height_ << "," << hitbox.pos_.x << "," << hitbox.pos_.y << "," << hitbox.pos_.z << "," << hitbox.layer_ << "}";
	}
	file_ << ']';//end of hitbox data for frame
}

void FighterFileExporter::addAnimationDataToFile(const std::string& AnimationName, const AnimationData& animationData)
{
	std::string animation = AnimationName + "Animation";
	std::string hitboxData = AnimationName + "HitboxData";

	ADD_TO_FILE(animation) << animationData.animationName.c_str() << std::endl;
	ADD_TO_FILE(hitboxData);
	for (const std::vector<Hitbox>& hitboxFrame : animationData.hitboxData)
	{
		AddHitboxToFile(hitboxFrame);
	}
	std::endl(file_);
}

void FighterFileExporter::populateFile(const ExportData& exportData)
{
	ADD_TO_FILE("ModelPath") << exportData.modelFilePath.c_str() << std::endl;
	ADD_TO_FILE("TexturePath") << exportData.textureFilePath.c_str() << std::endl;
	ADD_TO_FILE("RightSideRotation") << exportData.rightSideRotation << std::endl;
	ADD_TO_FILE("UpRotation") << exportData.upRotation << std::endl;
	ADD_TO_FILE("FlipHitboxes") << 0 << std::endl;
	addAnimationDataToFile("Idle", exportData.idleData);
	addAnimationDataToFile("Walk", exportData.walkData);
	addAnimationDataToFile("Crouch", exportData.crouchData);
	addAnimationDataToFile("Jump", exportData.jumpData);
	addAnimationDataToFile("Hit", exportData.hitData);
	addAnimationDataToFile("Block", exportData.blockData);
	for (int i = 0; i < exportData.attacks.size(); i++)
	{
		ADD_TO_FILE("animationName") << exportData.attacks[i].animationName.c_str() << std::endl;
		ADD_TO_FILE("startup") << exportData.attacks[i].startup << std::endl;
		ADD_TO_FILE("active") << exportData.attacks[i].active << std::endl;
		ADD_TO_FILE("recovery") << exportData.attacks[i].recovery << std::endl;
		ADD_TO_FILE("blockstun") << exportData.attacks[i].blockStun << std::endl;
		ADD_TO_FILE("hitstun") << exportData.attacks[i].hitstun << std::endl;
		ADD_TO_FILE("pushMag") << exportData.attacks[i].pushMag << std::endl;
		ADD_TO_FILE("damage") << exportData.attacks[i].damage << std::endl;
		ADD_TO_FILE("type") << exportData.attacks[i].type << std::endl;
		ADD_TO_FILE("input") << (int)exportData.attacks[i].input[0] << std::endl;
		int numberOfFrames = exportData.attacks[i].startup + exportData.attacks[i].active + exportData.attacks[i].recovery;
		ADD_TO_FILE("NumFrames") << numberOfFrames << std::endl;
		for (int j = 0; j < numberOfFrames; j++)
		{
			AddHitboxToFile(exportData.attacks[i].hitboxes[j]);
		}
	}
}

#include <iostream>

#include "AnimationDataFileExporter.h"

#define ADD_TO_FILE(STRINGNAME) \
	file_ << STRINGNAME << " : "

AnimationDataFileExporter::AnimationDataFileExporter(const ExportData& exportData)
{
	char fileName[500];
	sprintf_s(fileName, "%s.fgAnim", exportData.attacks[0].animationName.c_str());
	file_.open (fileName, std::ofstream::out);
	if(file_.is_open())
	{
		ADD_TO_FILE("ModelPath") << exportData.modelFilePath.c_str() << std::endl;
		ADD_TO_FILE("TexturePath") << exportData.textureFilePath.c_str() << std::endl;
		ADD_TO_FILE("RightSideRotation") << exportData.rightSideRotation << std::endl;

		for (int i = 0; i < exportData.attacks.size(); i++)
		{
			ADD_TO_FILE("animationName") << exportData.attacks[i].animationName.c_str() << std::endl;
			ADD_TO_FILE("startup") << exportData.attacks[i].startup << std::endl;
			ADD_TO_FILE("active") << exportData.attacks[i].active << std::endl;
			ADD_TO_FILE("recovery") << exportData.attacks[i].recovery << std::endl;
			ADD_TO_FILE("blockstun") << exportData.attacks[i].blockStun << std::endl;
			ADD_TO_FILE("hitstun") << exportData.attacks[i].hitstun << std::endl;
			ADD_TO_FILE("damage") << exportData.attacks[i].damage << std::endl;
			int numberOfFrames = exportData.attacks[i].startup + exportData.attacks[i].active + exportData.attacks[i].recovery;
			ADD_TO_FILE("NumFrames") << numberOfFrames << std::endl;
			for (int j = 0; j < numberOfFrames; j++)
			{
				AddHitboxToFile(exportData.attacks[i].hitboxes[j]);
			}
		}
	}
	else
	{
		std::cout << "cant open file" << std::endl;
	}

}

//Store hitboxes in this format [{width,height,pos.x,pos.y,pos.z,layer}{...}....]
void AnimationDataFileExporter::AddHitboxToFile(const std::vector<Hitbox>& hitboxes)
{
	file_ << '[';//start of hitbox data
	for (const Hitbox& hitbox : hitboxes)
	{
		file_ << "{" << hitbox.width_ << "," << hitbox.height_ << "," << hitbox.pos_.x << "," << hitbox.pos_.y << "," << hitbox.pos_.z << "," << hitbox.layer_ << "}";
	}
	file_ << ']';//end of hitbox data
}

AnimationDataFileExporter::~AnimationDataFileExporter()
{
	file_.close();
}


#include <iostream>
#include <string.h>

#include "FighterFileImporter.h"


static int getSizeOfString(const std::string& string)
{
	return string.size();
}

FighterFileImporter::FighterFileImporter(const std::string& filePath)
{
	file_.open(filePath, std::ios::in);
	if (file_.is_open())
	{
		readFileNew();
	}
	else
	{
		std::cout << "couldn't open file" << std::endl;
	}
}

std::vector<FrameInfo> FighterFileImporter::extraAttackColliderData(std::string hitboxData)
{
	std::vector<FrameInfo> frameData;
	for (std::string::iterator character = hitboxData.begin(); character < hitboxData.end(); character++)
	{
		if (*character == '[')
		{
			//frame info
			//(displacment.x, displacement.y, displacement.z, tags, type)
			FrameInfo frameInfo;
			if (*(character+1) == '(')
			{
				character++;
				for (int i = 0; i < 5; i++)
				{
					std::string currNum;
					while (*(++character) != ',' && *character != ')')
					{
							currNum.push_back(*character);
					}
					switch (i)
					{
					case 0:
						frameInfo.displacement_.x = stof(currNum);
						break;
					case 1:
						frameInfo.displacement_.y = stof(currNum);
						break;
					case 2:
						frameInfo.displacement_.z = stof(currNum);
						break;
					case 3: 
						frameInfo.tags_ = stoi(currNum);
						break;
					case 4: 
						frameInfo.type_ = (FrameType)stoi(currNum);
						break;
					}
				}
			}
			std::vector<BoxCollider> colliderData;
			while (*(++character) != ']')
			{
				if (*character == '{') 
				{
					BoxCollider currentCollider;
					//loops through each field needed to populate collider data
					//{size.x, size.y, size.z, pos.x, pos.y, pos.z, layer}
					for (int currentField = 0; currentField < 7; currentField++)
					{
						std::string currNum;
						while (*(++character) != ',' && *character != '}')
						{
							currNum.push_back(*character);
						}
						switch (currentField)
						{
							case 0:
							{
								currentCollider.size_.x = std::stof(currNum);
								break;
							}
							case 1:
							{
								currentCollider.size_.y = std::stof(currNum);
								break;
							}
							case 2:
							{
								currentCollider.size_.z = std::stof(currNum);
								break;
							}
							case 3:
							{
								currentCollider.position_.x = std::stof(currNum);
								break;
							}
							case 4:
							{
								currentCollider.position_.y = std::stof(currNum);
								break;
							}
							case 5:
							{
								currentCollider.position_.z = std::stof(currNum);
								break;
							}
							case 6:
							{
								currentCollider.layer_ = (HitboxLayers)std::stoi(currNum);
								break;
							}
						}
						currNum.clear();
					}
					colliderData.push_back(currentCollider);
				}
			}
			frameInfo.colliders_ = colliderData;
			frameData.push_back(frameInfo);
		}
	}
	return frameData;
}

std::vector<uint8_t> FighterFileImporter::extractInput(std::string inputData)
{
	std::vector<uint8_t> output;
	std::string::iterator character = inputData.begin();
	if (*character == '{')
	{
		std::string num;
		while (*(++character) != '}')
		{
			if (*character == ',')
			{
				output.push_back(stoi(num));
			}
			num += *character;
		}
		output.push_back(stoi(num));
		return output;
	}
	else
	{
		std::cout << "Error extract input failed" << std::endl;
	}

	return std::vector<uint8_t>();
}

FighterFileImporter::~FighterFileImporter()
{

}

void FighterFileImporter::readFileNew()
{
	std::string currentLine;
	//get modelPath
	if (std::getline(file_, currentLine))
	{
		exportData_.modelFilePath = currentLine.substr(12);
	}

	//get texturepath
	if (std::getline(file_, currentLine))
	{
		exportData_.textureFilePath = currentLine.substr(14);
	}

	//get idleAnimation name
	if (std::getline(file_, currentLine))
	{
		exportData_.idleData.animationName = currentLine.substr(getSizeOfString("IdleAnimation : "));
	}

	//get idleHitboxData
	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("IdleHitboxData : ")));
		exportData_.idleData.frameData = frameData;
	}

	//get WalkAnimation name
	if (std::getline(file_, currentLine))
	{
		exportData_.walkingData.animationName = currentLine.substr(getSizeOfString("WalkAnimation : "));
	}

	//get walkHitboxData 
	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("WalkHitboxData : ")));
		exportData_.walkingData.frameData = frameData;
	}

	//crouch
	if (std::getline(file_, currentLine))
	{
		exportData_.crouchData.animationName = currentLine.substr(getSizeOfString("CrouchAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("CrouchHitboxData : ")));
		exportData_.crouchData.frameData = frameData;
	}

	//jump data
	if (std::getline(file_, currentLine))
	{
		exportData_.jumpData.animationName = currentLine.substr(getSizeOfString("JumpAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("JumpHitboxData : ")));
		exportData_.jumpData.frameData = frameData;
	}

	//hit data
	if (std::getline(file_, currentLine))
	{
		exportData_.hitData.animationName = currentLine.substr(getSizeOfString("HitAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("HitHitboxData : ")));
		exportData_.hitData.frameData = frameData;
	}

	//block data
	if (std::getline(file_, currentLine))
	{
		exportData_.blockData.animationName = currentLine.substr(getSizeOfString("BlockAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("BlockHitboxData : ")));
		exportData_.blockData.frameData = frameData;
	}

	while (std::getline(file_, currentLine))
	{
		AttackData attack{};
		if (currentLine.find("animationName : ") != std::string::npos)
		{
			attack.animationName_ = currentLine.substr(getSizeOfString("animationName : "));
			if (std::getline(file_, currentLine))
			{
				attack.startupFrames_ =  std::stoi(currentLine.substr(getSizeOfString("startup : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.activeFrames_ = std::stoi(currentLine.substr(getSizeOfString("active : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.recoveryFrames_ = std::stoi(currentLine.substr(getSizeOfString("recovery : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.blockstun_ = std::stoi(currentLine.substr(getSizeOfString("blockstun : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.hitstun_ = std::stoi(currentLine.substr(getSizeOfString("hitstun : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.freezeFrames_ = std::stoi(currentLine.substr(getSizeOfString("freeze : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.push_ = std::stof(currentLine.substr(getSizeOfString("pushMag : ")));
			}
			if (std::getline(file_, currentLine))
			{
				attack.damage_ = std::stoi(currentLine.substr(getSizeOfString("damage : ")));
			}
			std::vector<uint8_t> attackInput;
			if (std::getline(file_, currentLine))
			{
				attackInput = extractInput(currentLine.substr(getSizeOfString("input : ")));
			}
			attack.inputs_ = attackInput;

			if (std::getline(file_, currentLine))
			{
				std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("AttackData : ")));;
				attack.frameData_ = frameData;
			}
		}
		exportData_.attackData.push_back(attack);
	}
}


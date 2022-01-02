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
		readFile();
	}
	else
	{
		std::cout << "couldn't open file" << std::endl;
	}
}

std::vector<std::vector<Hitbox>> FighterFileImporter::extractHitboxData(std::string hitboxData)
{
	std::vector<std::vector<Hitbox>> returnData;
	for (std::string::iterator character = hitboxData.begin(); character < hitboxData.end(); character++)
	{
		if (*character == '[')
		{
			std::vector<Hitbox> frameData;
			while (*(++character) != ']')
			{
				if (*character == '{')
				{
					Hitbox h{};
					//loops through each field needed to populate hitbox data
					for (int currentField = 0; currentField < 6; currentField++)
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
								h.width_ = std::stof(currNum);
								break;
							}
							case 1:
							{
								h.height_ = std::stof(currNum);
								break;
							}
							case 2:
							{
								h.position_.x = std::stof(currNum);
								break;
							}
							case 3:
							{
								h.position_.y = std::stof(currNum);
								if (flipHitboxes_)
									h.position_.y *= -1;
								break;
							}
							case 4:
							{
								h.position_.z = std::stof(currNum);
								break;
							}
							case 5:
							{
								h.layer_ = (Hitbox::HitboxLayer)std::stoi(currNum);
								break;
							}
						}
						currNum.clear();

						}
					frameData.push_back(h);
					}
				}
				returnData.push_back(frameData);
			}
		}
	return returnData;
}

//This will populate exportData_
void FighterFileImporter::readFile()
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
	//get rightside rotation
	if (std::getline(file_, currentLine))
	{
		exportData_.rightSideRotation = std::stof(currentLine.substr(20));
	}

	//get up rotation
	if (std::getline(file_, currentLine))
	{
		exportData_.upRotation = std::stof(currentLine.substr(getSizeOfString("UpRotation : ")));
	}

	//get FlipHitboxes flag
	if (std::getline(file_, currentLine))
	{
		flipHitboxes_ = std::stoi(currentLine.substr(getSizeOfString("FlipHitboxes : ")));
	}

	//get idleAnimation name
	if (std::getline(file_, currentLine))
	{
		exportData_.idleData.animationName = currentLine.substr(getSizeOfString("IdleAnimation : "));
	}

	//get idleHitboxData
	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("IdleHitboxData : ")));
		exportData_.idleData.hitboxData = extractedHitboxData;
	}

	//get WalkAnimation name
	if (std::getline(file_, currentLine))
	{
		exportData_.walkingData.animationName = currentLine.substr(getSizeOfString("WalkAnimation : "));
	}

	//get walkHitboxData 
	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("WalkHitboxData : ")));
		exportData_.walkingData.hitboxData = extractedHitboxData;
	}

	if (std::getline(file_, currentLine))
	{
		exportData_.crouchData.animationName = currentLine.substr(getSizeOfString("CrouchAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("CrouchHitboxData : ")));
		exportData_.crouchData.hitboxData = extractedHitboxData;
	}

	if (std::getline(file_, currentLine))
	{
		exportData_.jumpData.animationName = currentLine.substr(getSizeOfString("JumpAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("JumpHitboxData : ")));
		exportData_.jumpData.hitboxData = extractedHitboxData;
	}

	if (std::getline(file_, currentLine))
	{
		exportData_.hitData.animationName = currentLine.substr(getSizeOfString("HitAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("HitHitboxData : ")));
		exportData_.hitData.hitboxData = extractedHitboxData;
	}

	if (std::getline(file_, currentLine))
	{
		exportData_.blockData.animationName = currentLine.substr(getSizeOfString("BlockAnimation : "));
	}

	if(std::getline(file_, currentLine))
	{ 
		std::vector<std::vector<Hitbox>> extractedHitboxData = extractHitboxData(currentLine.substr(getSizeOfString("BlockHitboxData : ")));
		exportData_.blockData.hitboxData = extractedHitboxData;
	}

	while (std::getline(file_, currentLine))
	{
		Attack attack{};
		if (currentLine.find("animationName : ") != std::string::npos)
		{
			attack.animationName_ = currentLine.substr(16);
			if (std::getline(file_, currentLine))
			{
				attack.startupFrames = std::stoi(currentLine.substr(10));
			}
			if (std::getline(file_, currentLine))
			{
				attack.activeFrames = std::stoi(currentLine.substr(9));
			}
			if (std::getline(file_, currentLine))
			{
				attack.recoveryFrames = std::stoi(currentLine.substr(11));
			}
			if (std::getline(file_, currentLine))
			{
				attack.blockstunFrames = std::stoi(currentLine.substr(12));
			}
			if (std::getline(file_, currentLine))
			{
				attack.hitstunFrames = std::stoi(currentLine.substr(10));
			}
			if (std::getline(file_, currentLine))
			{
				attack.hitPushMag = std::stof(currentLine.substr(10));
			}
			if (std::getline(file_, currentLine))
			{
				attack.damage = std::stoi(currentLine.substr(9));
			}
			std::vector<InputKey> attackInput;
			if (std::getline(file_, currentLine))
			{
				attackInput.push_back(std::stoi(currentLine.substr(8)));
			}
			exportData_.inputData.push_back(attackInput);
			int numFrames = 0;
			if (std::getline(file_, currentLine))
			{
				numFrames = std::stoi(currentLine.substr(12));
			}

			attack.hurtboxWidthHeight.resize(numFrames);
			attack.hurtboxPos.resize(numFrames);
			if (std::getline(file_, currentLine))
			{
				AnimationData parsedAttackData{};
				for (std::string::iterator character = currentLine.begin(); character < currentLine.end(); character++)
				{
					if (*character == '[')
					{
						std::vector<Hitbox> frameData;
						while (*(++character) != ']')
						{
							if (*character == '{')
							{
								Hitbox h{};
								//loops through each field needed to populate hitbox data
								for (int currentField = 0; currentField < 6; currentField++)
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
											h.width_ = std::stof(currNum);
											break;
										}
										case 1:
										{
											h.height_ = std::stof(currNum);
											break;
										}
										case 2:
										{
											h.position_.x = std::stof(currNum);
											break;
										}
										case 3:
										{
											h.position_.y = std::stof(currNum);
											if (flipHitboxes_)
												h.position_.y *= -1;
											break;
										}
										case 4:
										{
											h.position_.z = std::stof(currNum);
											break;
										}
										case 5:
										{
											h.layer_ = (Hitbox::HitboxLayer)std::stoi(currNum);
											break;
										}
									}
									currNum.clear();

									}
								frameData.push_back(h);
								}
							}
							parsedAttackData.hitboxData.push_back(frameData);
							attack.hitboxesPerFrame.push_back(frameData);
						}
					}
				exportData_.attackData.push_back(parsedAttackData);
				}
			}
		exportData_.attacks.push_back(attack);
		}
}

FighterFileImporter::~FighterFileImporter()
{

}

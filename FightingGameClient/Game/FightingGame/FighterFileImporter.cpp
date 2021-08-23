#include <iostream>
#include <string.h>

#include "FighterFileImporter.h"

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
				attack.damage = std::stoi(currentLine.substr(9));
			}
			int numFrames = 0;
			if (std::getline(file_, currentLine))
			{
				numFrames = std::stoi(currentLine.substr(12));
			}

			attack.hurtboxWidthHeight.resize(numFrames);
			attack.hurtboxPos.resize(numFrames);
			if (std::getline(file_, currentLine))
			{
				AttackData parsedAttackData{};
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
											h.pos_.x = std::stof(currNum);
											break;
										}
										case 3:
										{
											h.pos_.y = std::stof(currNum);
											break;
										}
										case 4:
										{
											h.pos_.z = std::stof(currNum);
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

#include <iostream>
#include <string.h>
#include <algorithm>

#include "FighterFileImporter.h"

static std::string strCollLayers[NUM_LAYERS] =
{
	"PUSH_BOX",
	"HURT_BOX",
	"HIT_BOX"
};

static std::unordered_map<std::string, FrameTags> strToTagMap = 
{
	{"INVINCIBLE", FrameTags::INVINCIBLE},
	{"CANCELABLE",  FrameTags::CANCELABLE},
	{"COUNTER_HIT", FrameTags::COUNTER_HIT},
	{"SET_VELOCITY", FrameTags::SET_VELOCITY},
	{"EXECUTE_CANCEL", FrameTags::EXECUTE_CANCEL}
};

static std::unordered_map<std::string, FightingGameInput::Action> strToInput = 
{
	{"NOTHING",FightingGameInput::Action::nothing},
	{"LEFT"   ,FightingGameInput::Action::left},
	{"RIGHT"  ,FightingGameInput::Action::right},
	{"DOWN"   ,FightingGameInput::Action::down},
	{"UP"     ,FightingGameInput::Action::up},
	{"LIGHT"  ,FightingGameInput::Action::light},
	{"MEDIUM" ,FightingGameInput::Action::medium},
	{"STRONG" ,FightingGameInput::Action::strong},
	{"ULTRA"  ,FightingGameInput::Action::ultra}
};

static std::unordered_map<std::string, MoveState> strToMoveState = 
{
	{"STANDING", MoveState::STANDING},
	{"CROUCHING", MoveState::CROUCHING},
	{"JUMPING", MoveState::JUMPING}
};

//not using a map for this since this is only called while initally reading files and array is only 3 elements
ColliderLayer stringToColliderLayer(std::string colliderLayer)
{
	for (int i = 0; i < NUM_LAYERS; i++)
	{
		if (strCollLayers[i] == colliderLayer)
		{
			return ColliderLayer(i);
		}
	}
}

FrameTags stringToTag(std::string tag)
{
	return strToTagMap[tag];
}

FightingGameInput::Action stringToAction(std::string str)
{
	return strToInput[str];
}


static int getSizeOfString(const std::string& string)
{
	return string.size();
}

FighterFileImporter::FighterFileImporter(const std::string& filePath)
{
	file_.open(filePath, std::ios::in);
	if (file_.is_open())
	{
		readFileNew(filePath);
		//readFile();
		//extractAttackDataFile("C:\\Users\\jsngw\\source\\repos\\FightingGame\\FighterFiles\\NewFighter\\AttackFileDraft.attck");
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
						frameInfo.velocity_.x = stof(currNum);
						break;
					case 1:
						frameInfo.velocity_.y = stof(currNum);
						break;
					case 2:
						//frameInfo.velocity_.z = stof(currNum);
						break;
					case 3: 
						frameInfo.frameTags_ = stoi(currNum);
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
								currentCollider.layer_ = (ColliderLayer)std::stoi(currNum);
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

void FighterFileImporter::readFileNew(std::string filepath)
{
	std::ifstream file;
	std::string currentLine;

	file.open(filepath, std::ios::in);
	if (!file.is_open())
	{
		std::cout << "Could not open file" << std::endl;
		assert(false);
	}

	//get modelPath
	if (std::getline(file, currentLine))
	{
		exportData_.modelFilePath = currentLine.substr(12);
	}

	//get texturepath
	if (std::getline(file, currentLine))
	{
		exportData_.textureFilePath = currentLine.substr(14);
	}

	//get idleAnimation name
	if (std::getline(file, currentLine))
	{
		exportData_.idleData.animationName = currentLine.substr(getSizeOfString("IdleAnimation : "));
	}

	//get idleHitboxData
	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("IdleHitboxData : ")));
		exportData_.idleData.frameData = frameData;
	}

	//get WalkAnimation name
	if (std::getline(file, currentLine))
	{
		exportData_.walkingData.animationName = currentLine.substr(getSizeOfString("WalkAnimation : "));
	}

	//get walkHitboxData 
	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("WalkHitboxData : ")));
		exportData_.walkingData.frameData = frameData;
	}

	//crouch
	if (std::getline(file, currentLine))
	{
		exportData_.crouchData.animationName = currentLine.substr(getSizeOfString("CrouchAnimation : "));
	}

	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("CrouchHitboxData : ")));
		exportData_.crouchData.frameData = frameData;
	}

	//jump data
	if (std::getline(file, currentLine))
	{
		exportData_.jumpData.animationName = currentLine.substr(getSizeOfString("JumpAnimation : "));
	}

	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("JumpHitboxData : ")));
		exportData_.jumpData.frameData = frameData;
	}

	//hit data
	if (std::getline(file, currentLine))
	{
		exportData_.hitData.animationName = currentLine.substr(getSizeOfString("HitAnimation : "));
	}

	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("HitHitboxData : ")));
		exportData_.hitData.frameData = frameData;
	}

	//block data
	if (std::getline(file, currentLine))
	{
		exportData_.blockData.animationName = currentLine.substr(getSizeOfString("BlockAnimation : "));
	}

	if(std::getline(file, currentLine))
	{ 
		std::vector<FrameInfo> frameData = extraAttackColliderData(currentLine.substr(getSizeOfString("BlockHitboxData : ")));
		exportData_.blockData.frameData = frameData;
	}

	while (std::getline(file, currentLine))
	{
		size_t pos = 0;
		if ((pos = currentLine.find("Move")) != std::string::npos)
		{
			std::string filepath = extractSingleParen(currentLine, "Move");
			//std::string filepath = currentLine.substr(pos + sizeof("Move:"));
			MoveInfo move = extractAttackDataFile(filepath);
			exportData_.moveData.push_back(move);
		}
	}
}

std::string FighterFileImporter::extractSingleParen(std::string line, std::string descriptor)
{
	size_t index = line.find(descriptor);
	if (index != std::string::npos)
	{
		size_t firstOccurence = line.substr(index).find('(');
		std::string parse = line.substr(index).substr(firstOccurence);
		std::string::iterator parseIt = parse.begin();

		std::string returnVal;
		while (++parseIt != parse.end() && *parseIt != ')' )
		{
			returnVal.push_back(*parseIt);
		}
		return returnVal;
	}
	else
	{
		std::cout << "Error descriptor not found.  Returning empty string" << std::endl;
		return "";
	}
}

float FighterFileImporter::extractSingleParenInt(std::string line, std::string descriptor)
{
	size_t index = line.find(descriptor);
	if (index != std::string::npos)
	{
		size_t firstOccurence = line.substr(index).find('(');
		std::string parse = line.substr(index).substr(firstOccurence);
		std::string::iterator parseIt = parse.begin();

		std::string returnVal;
		while (++parseIt != parse.end() && *parseIt != ')' )
		{
			returnVal.push_back(*parseIt);
		}
		return std::stof(returnVal);
	}
	else
	{
		std::cout << "Error descriptor not found.  Returning empty string" << std::endl;
		return -1;
	}
}


glm::vec3 FighterFileImporter::extractTripleParenVec(std::string line, std::string descriptor)
{
	size_t index = line.find(descriptor);
	if (index != std::string::npos)
	{
		size_t firstOccurence = line.substr(index).find('(');
		std::string parse = line.substr(index).substr(firstOccurence);
		std::string::iterator parseIt = parse.begin();

		glm::vec3 returnVal;
		std::string strToInt;
		int valIndex = 0;
		while (++parseIt != parse.end() && *parseIt != ')' )
		{
			if (*parseIt == ',')
			{
				returnVal[valIndex] = stof(strToInt);
				strToInt.clear();
				valIndex++;
			}
			else
			{
				strToInt.push_back(*parseIt);
			}
		}
		returnVal[valIndex] = stof(strToInt);
		return returnVal;
	}
	else
	{
		std::cout << "Error descriptor not found.  Returning empty string" << std::endl;
		return glm::vec3(0.0f);
	}
}

std::vector<std::string> FighterFileImporter::extractParenStr(std::string line, std::string descriptor)
{
	size_t index = line.find(descriptor);
	if (index != std::string::npos)
	{
		size_t firstOccurence = line.substr(index).find('(');
		std::string parse = line.substr(index).substr(firstOccurence);
		std::string::iterator parseIt = parse.begin();

		std::vector<std::string> returnVal;
		std::string str;
		while (++parseIt != parse.end() && *parseIt != ')' )
		{
			if (*parseIt == ',')
			{
				for (int i = 0; i < str.size(); i++)
				{
					if (str[i] == ' ')
					{
						str.erase(str.begin() + i);
					}
				}
				returnVal.push_back(str);
				str.clear();
			}
			else
			{
				str.push_back(*parseIt);
			}
		}

		for (int i = 0; i < str.size(); i++)
		{
			if (str[i] == ' ')
			{
				str.erase(str.begin() + i);
			}
		}
		returnVal.push_back(str);
		return returnVal;
	}
	else
	{
		std::cout << "Error descriptor not found.  Returning empty string" << std::endl;
		return std::vector<std::string>();
	}
}

std::vector<int> FighterFileImporter::extractParenInt(std::string line, std::string descriptor)
{
	size_t index = line.find(descriptor);
	if (index != std::string::npos)
	{
		size_t firstOccurence = line.substr(index).find('(');
		std::string parse = line.substr(index).substr(firstOccurence);
		std::string::iterator parseIt = parse.begin();

		std::vector<int> returnVal;
		std::string str;
		while (++parseIt != parse.end() && *parseIt != ')' )
		{
			if (*parseIt == ',')
			{
				returnVal.push_back(stoi(str));
				str.clear();
			}
			else
			{
				str.push_back(*parseIt);
			}
		}

		for (int i = 0; i < str.size(); i++)
		{
			if (str[i] == ' ')
			{
				str.erase(str.begin() + i);
			}
		}
		returnVal.push_back(stoi(str));
		return returnVal;
	}
	else
	{
		std::cout << "Error descriptor not found.  Returning empty string" << std::endl;
		return std::vector<int>();
	}
}


BoxCollider FighterFileImporter::extractColliderData(std::ifstream& file)
{
	//Example Format of collider field
    //Collider
    //{
    //    layer(PUSH_BOX)
    //    size(1, 2, 1)
    //    pos(0, 0, 0)
    //}
	// order MUST BE LAYER then SIZE then POS 

	std::string line;
	std::getline(file, line);
	BoxCollider collider;
	if (line.find('{') != std::string::npos)
	{
		std::getline(file, line);
		std::string enumName  = extractSingleParen(line, "layer");
		collider.layer_ = stringToColliderLayer(enumName);

		std::getline(file, line);
		collider.size_ = extractTripleParenVec(line, "size");

		std::getline(file, line);
		collider.position_ = extractTripleParenVec(line, "pos");
	}
	else
	{
		std::cout << "There must be a { } containg all collider data" << std::endl;
	}
	return collider;
}

int FighterFileImporter::extractTags(std::string line)
{
	std::vector<std::string> tagsStr = extractParenStr(line, "Tags");
	int tags = 0;

	for (int i = 0; i < tagsStr.size(); i++)
	{
		int tag = stringToTag(tagsStr[i]);
		tags |= tag;
	}
	return tags;
}

MoveInfo FighterFileImporter::extractAttackDataFile(std::string filePath)
{
	MoveInfo moveInfo;
	std::ifstream attackFile;
	std::string currentLine = "";

	attackFile.open(filePath, std::ios::in);
	assert(attackFile.is_open());//make sure file exists
	//first line will always be attack name
	//std::getline(attackFile, currentLine);
	//moveInfo.moveName_ = currentLine;
	//second line will always be animation name
	//std::getline(attackFile, currentLine);
	//moveInfo.animationName_ = currentLine;
	int currentFrame = 0;
	int numFrames = 0;

	std::getline(attackFile, currentLine);
	moveInfo.name_ = currentLine;
	//before frame data extract all attack data
	while (currentLine.find('}') == std::string::npos) 
	{
		if (currentLine.find("Input") != std::string::npos)
		{
			std::vector<std::string> actionsStr;
			std::vector<uint8_t> input;
			actionsStr = extractParenStr(currentLine, "Input");
			for(int i = 0; i < actionsStr.size(); i++)
			{
				input.push_back(stringToAction(actionsStr[i]));
			}
			exportData_.inputs.push_back(input);
		}
		else if (currentLine.find("Damage") != std::string::npos)
		{
			moveInfo.hitEffect_.damage_ = extractSingleParenInt(currentLine, "Damage");
		}
		else if (currentLine.find("Hitstun") != std::string::npos)
		{
			moveInfo.hitEffect_.hitstun_ = extractSingleParenInt(currentLine, "Hitstun");
		}
		else if (currentLine.find("Blockstun") != std::string::npos)
		{
			moveInfo.hitEffect_.blockstun_ = extractSingleParenInt(currentLine, "Blockstun");
		}
		else if (currentLine.find("Push") != std::string::npos)
		{
			moveInfo.hitEffect_.pushBack_ = extractSingleParenInt(currentLine, "Push");
		}
		else if (currentLine.find("Freeze") != std::string::npos)
		{
			moveInfo.hitEffect_.freezeFrames_ = extractSingleParenInt(currentLine, "Freeze");
		}
		else if (currentLine.find("AnimationName") != std::string::npos)
		{
			moveInfo.animationName_ = extractSingleParen(currentLine, "AnimationName");
		}
		else if (currentLine.find("MoveState") != std::string::npos)
		{
			std::string stateStr = extractSingleParen(currentLine, "MoveState");
			auto iter = strToMoveState.find(stateStr);
			_ASSERT(iter != strToMoveState.end());
			moveInfo.moveState_ = iter->second;
		}
		else if (currentLine.find("CancelList") != std::string::npos)
		{
			auto pos = currentLine.find('[');
			std::string idStr = currentLine.substr(pos + 1, 1);
			int id = std::stoi(idStr);
			std::vector<std::string> cancelList = extractParenStr(currentLine, "CancelList");
			moveInfo.cancelMap_.insert({ id, cancelList });
		}
		std::getline(attackFile, currentLine);
	}

	while (std::getline(attackFile, currentLine))
	{
		FrameInfo frameInfo;
		int extend = 1;
		int range[2] = {currentFrame, currentFrame+1};
		//start of frame
		if (currentLine == "[")
		{
			while (currentLine != "]")
			{
				std::getline(attackFile, currentLine);
				size_t pos ;
				if (currentLine.find("Collider") != std::string::npos)
				{
					frameInfo.colliders_.push_back(extractColliderData(attackFile));
				}
				else if (currentLine.find("Tags") != std::string::npos)
				{
					frameInfo.frameTags_ = extractTags(currentLine);
				}
				else if (currentLine.find("Extends") != std::string::npos)
				{
					int extend = extractSingleParenInt(currentLine, "Extends");
					range[1] += extend;
				}
				else if (currentLine.find("Cancelable") != std::string::npos)
				{
					frameInfo.cancelListIds_ = extractParenInt(currentLine, "Cancelable");
					frameInfo.frameTags_ |= FrameTags::CANCELABLE;
				}
				else if (currentLine.find("SetVelocity") != std::string::npos)
				{
					std::vector<int> velocity = extractParenInt(currentLine, "SetVelocity");
					_ASSERT(velocity.size() == 2);
					frameInfo.velocity_ = glm::vec2(velocity[0], velocity[1]);
					frameInfo.frameTags_ |= FrameTags::SET_VELOCITY;
					if (frameInfo.velocity_.y > 0)
					{
						frameInfo.frameTags_ |= FrameTags::TRANSITION_TO_AERIAL;
					}
				}
				else if (currentLine.find("Range") != std::string::npos)
				{
					std::vector<int> rangeVec = extractParenInt(currentLine, "Range");
					_ASSERT(rangeVec.size() == 2);
					_ASSERT(rangeVec[0] <= numFrames);
					memcpy(range, rangeVec.data(), sizeof(int) * 2);
				}
			}

			for (int i = range[0]; i < range[1]; i++)
			{
				if(i < moveInfo.frameData_.size())
				{
					moveInfo.frameData_[i].colliders_.insert(moveInfo.frameData_[i].colliders_.begin(), frameInfo.colliders_.begin(), frameInfo.colliders_.end());
					moveInfo.frameData_[i].cancelListIds_.insert(moveInfo.frameData_[i].cancelListIds_.begin(), frameInfo.cancelListIds_.begin(), frameInfo.cancelListIds_.end());
					moveInfo.frameData_[i].frameTags_ |= frameInfo.frameTags_;
					moveInfo.frameData_[i].blockHeight_ |= frameInfo.blockHeight_;
					moveInfo.frameData_[i].attackHeight_ |= frameInfo.attackHeight_;
					moveInfo.frameData_[i].velocity_ = frameInfo.velocity_;
				}
				else
				{
					moveInfo.frameData_.push_back(frameInfo);
				}
			}
			currentFrame++;
			numFrames = moveInfo.frameData_.size();
		}
	}

	attackFile.close();
	return moveInfo;
}


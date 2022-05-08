#pragma once
#include <unordered_map>
#include <list>
#include <queue>
#include <stack>

#include "glm/glm.hpp"
#include "../Events.h"
#include "../../Engine/EngineExport.h"

class Application;
struct ImGuiInputTextCallbackData;

enum CommandType
{
	CommandType_bool,
	CommandType_float,
	CommandType_int,
	CommandType_vec3,
	CommandType_function,
};

union CommandData
{
	bool boolData;
	float floatData;
	int intData;
	glm::vec3 vec3Data;
};

struct CommandVar;

typedef std::function<void(std::string command, CommandVar* commandVar)> CallbackFunc;

struct CommandVar
{
	CommandType type;
	CommandData data;
	CallbackFunc func;
};

class ENGINE_API Console
{
public:

	Console(Application* application);
	~Console() = default;

	static Console* getInstance() { return instance_; };
	static void setInstance(Console* console) { instance_ = console; }
	static void destroyInstance() { delete instance_; instance_ = nullptr; }

	void handleEvent(Events::Event& currEvent);
	void handleKeyPressedEvent(Events::KeyPressedEvent& keyPressedEvent);

	static void addFloatVar(std::string command, float variableDefault, CallbackFunc func = nullptr);
	static float getFloatVar(std::string comamnd);
	static void addIntVar(std::string command, int variableDefault, CallbackFunc func = nullptr);
	static int getIntVar(std::string command);
	static void addBoolVar(std::string command, bool variableDefault, CallbackFunc func = nullptr);
	static bool getBoolVar(std::string command);
	static void addVec3Var(std::string command, glm::vec3 variableDefault, CallbackFunc func = nullptr);
	static glm::vec3 getVec3Var(std::string command);

	void update();

	void inputTextCallback(ImGuiInputTextCallbackData* data);


private:
	void drawConsole();
	void getAllCandidates(ImGuiInputTextCallbackData* data);
	

	static Console* instance_;
	bool shouldDraw_ = false;
	std::vector<std::string> candidates_;//candidates for likely command matches
	static std::unordered_map<std::string, CommandVar> commandDataMap_;
	int currCandidate_;
#define HISTORY_SIZE 3 
	std::string history_[HISTORY_SIZE] = { "" };
	int currHistory_ = 0;
	int historyHead_ = 0;

	std::list<std::string> consoleOutput_;

};
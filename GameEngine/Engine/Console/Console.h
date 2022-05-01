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
	void(*funcData)(void);
};

struct CommandVar;

typedef std::function<void(std::string command, CommandVar* commandVar)> CallbackFunc;

struct CommandVar
{
	CommandType type;
	CommandData data;
	CallbackFunc func = nullptr;
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

	void addFloatVar(std::string command, float variableDefault, CallbackFunc func = nullptr);
	float getFloatVar(std::string comamnd);
	void addIntVar(std::string command, int variableDefault, CallbackFunc func = nullptr);
	int getIntVar(std::string command);
	void addBoolVar(std::string command, bool variableDefault, CallbackFunc func = nullptr);
	bool getBoolVar(std::string command);
	void addVec3Var(std::string command, glm::vec3 variableDefault, CallbackFunc func = nullptr);
	glm::vec3 getVec3Var(std::string command);

	void update();

	void inputTextCallback(ImGuiInputTextCallbackData* data);

private:
	void drawConsole();
	void getAllCandidates(ImGuiInputTextCallbackData* data);
	

	static Console* instance_;
	bool shouldDraw_ = false;
	std::unordered_map<std::string, CommandVar> commandDataMap_;
	std::vector<std::string> candidates_;//candidates for likely command matches
	int currCandidate_;
#define HISTORY_SIZE 3 
	std::string history_[HISTORY_SIZE] = { "" };
	int currHistory_ = 0;
	int historyHead_ = 0;

	std::list<std::string> consoleOutput_;

};